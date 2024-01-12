use std::env;

fn main() -> miette::Result<()> {
	if let Ok(lib_dir) = env::var("ZXING_CPP_LIB_DIR") {
		println!("cargo:rustc-link-search=native={}", lib_dir);
		println!("cargo:rustc-link-lib=dylib=ZXing");
	}

	// manual bindings.rs generation:
	// bindgen <zxing-cpp.git/wrappers/c/zxing-c.h> -o src/bindings.rs --no-prepend-enum-name --merge-extern-blocks --use-core --allowlist-type "zxing.*" --allowlist-function "zxing.*"

	Ok(())
}
