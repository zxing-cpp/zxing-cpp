# pip install opencv-python numpy zxing-cpp httpx
from typing import Union
import sys
import cv2
import numpy as np
import zxingcpp
import httpx


def get_image_bytes(source: Union[str, bytes]) -> bytes:
    """Load image bytes from URL, file path, or bytes."""
    if isinstance(source, bytes):
        return source
    if source.startswith(("http://", "https://")):
        with httpx.Client() as client:
            response = client.get(source)
            response.raise_for_status()
            return response.content
    with open(source, "rb") as f:
        return f.read()


def scan_and_display(source: Union[str, bytes]):
    """Scan barcodes from image source and display results."""
    image_bytes = get_image_bytes(source)
    img_array = np.frombuffer(image_bytes, dtype=np.uint8)
    img = cv2.imdecode(img_array, cv2.IMREAD_COLOR)
    
    if img is None:
        raise ValueError("Failed to decode image")
    
    results = zxingcpp.read_barcodes(img)
    
    if not results:
        print("No barcode found")
        return
    
    for barcode in results:
        print(f"Text: {barcode.text}")
        print(f"Format: {barcode.format.name}")
        print(f"Content Type: {barcode.content_type.name}")
        
        pts = np.array([
            [barcode.position.top_left.x, barcode.position.top_left.y],
            [barcode.position.top_right.x, barcode.position.top_right.y],
            [barcode.position.bottom_right.x, barcode.position.bottom_right.y],
            [barcode.position.bottom_left.x, barcode.position.bottom_left.y]
        ], dtype=np.int32)
        
        cv2.polylines(img, [pts], isClosed=True, color=(0, 0, 255), thickness=3)
    
    scale = min(800 / img.shape[0], 1200 / img.shape[1], 1.0)
    if scale < 1.0:
        img = cv2.resize(img, (int(img.shape[1] * scale), int(img.shape[0] * scale)))
    
    cv2.imshow('Barcode Scan Result', img)
    cv2.waitKey(0)
    cv2.destroyAllWindows()


if __name__ == "__main__":
    if len(sys.argv) > 1:
        source = sys.argv[1]
    else:
        source = "https://upload.wikimedia.org/wikipedia/commons/thumb/3/39/Databar_14_00075678164125.png/250px-Databar_14_00075678164125.png"
    
    try:
        scan_and_display(source)
    except Exception as e:
        print(f"Error: {e}")
