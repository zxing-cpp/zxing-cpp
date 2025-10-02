from typing import List, Dict, Any, Union  # built-in
from PIL import Image, ImageDraw  # pip install pillow
from io import BytesIO  # built-in
import zxingcpp  # pip install zxing-cpp
import httpx  # pip install httpx

def _decode_image(data: bytes) -> List[Dict[str, Any]]:
    img = Image.open(BytesIO(data))
    results = zxingcpp.read_barcodes(img)
    if not results:
        raise ValueError("No barcode found.")
    return [{
        "code": barcode.text,
        "type": barcode.format.name,
        "content": barcode.content_type.name,
        "position": {
            "top_left": (barcode.position.top_left.x, barcode.position.top_left.y),
            "top_right": (barcode.position.top_right.x, barcode.position.top_right.y),
            "bottom_right": (barcode.position.bottom_right.x, barcode.position.bottom_right.y),
            "bottom_left": (barcode.position.bottom_left.x, barcode.position.bottom_left.y)
        }
    } for barcode in results]

def _get_bytes(source: Union[str, bytes]) -> bytes:
    try:
        if isinstance(source, bytes):
            return source
        if source.startswith(("http://", "https://")):
            with httpx.Client() as client:
                response = client.get(source)
                response.raise_for_status()
                return response.content
        with open(source, "rb") as f:
            return f.read()
    except Exception as e:
        raise RuntimeError(f"Failed to get bytes from source: {e}")

def scan_barcode(source: Union[str, bytes]) -> List[Dict[str, Any]]:
    """Scan barcode from image source synchronously."""
    data = _get_bytes(source)
    return _decode_image(data)

if __name__ == "__main__":
    import cv2, numpy as np  # pip install opencv-python numpy
    IMAGE_URL = "https://upload.wikimedia.org/wikipedia/commons/thumb/3/39/Databar_14_00075678164125.png/250px-Databar_14_00075678164125.png"

    def show_results(results, image_bytes):
        if not results:
            print("Hiç barkod bulunamadı."); return
        img = Image.open(BytesIO(image_bytes)).convert("RGB")
        draw = ImageDraw.Draw(img)
        for r in results:
            pts = [r['position'][k] for k in ("top_left", "top_right", "bottom_right", "bottom_left")]
            draw.polygon(pts, outline="red", width=4)
        arr = cv2.cvtColor(np.array(img), cv2.COLOR_RGB2BGR)
        scale = 800 / arr.shape[0]
        arr = cv2.resize(arr, (int(arr.shape[1] * scale), 800))
        cv2.imshow('Barkod Scan Result', arr)
        cv2.waitKey(0)
        cv2.destroyAllWindows()

    try:
        image_bytes = _get_bytes(IMAGE_URL)
        res = scan_barcode(image_bytes)
        print("Results:", res)
        show_results(res, image_bytes)
    except Exception as e:
        print(f"Scan error: {e}")
