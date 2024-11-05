use std::env;

fn main() -> miette::Result<()> {
	if cfg!(feature = "bundled") {
		// Builds the project in the directory located in `core`, installing it into $OUT_DIR
		let mut dst = cmake::Config::new("core")
			.define("BUILD_SHARED_LIBS", "OFF")
			.define("ZXING_READERS", "ON")
			.define("ZXING_WRITERS", "NEW")
			.define("ZXING_EXPERIMENTAL_API", "ON")
			.define("ZXING_C_API", "ON")
			.define("ZXING_USE_BUNDLED_ZINT", "ON")
			.define("CMAKE_CXX_STANDARD", "20")
			.build();
		dst.push("lib");
		println!("cargo:rustc-link-search=native={}", dst.display());
		println!("cargo:rustc-link-lib=static=ZXing");

		if let Ok(target) = env::var("TARGET") {
			if target.contains("apple") {
				println!("cargo:rustc-link-lib=dylib=c++");
			} else if target.contains("linux") {
				println!("cargo:rustc-link-lib=dylib=stdc++");
			}
		}
	} else if let Ok(lib_dir) = env::var("ZXING_CPP_LIB_DIR") {
		println!("cargo:rustc-link-search=native={}", lib_dir);
		println!("cargo:rustc-link-lib=dylib=ZXing");
	} else {
		// panic!("ZXing library not found. Use feature 'bundled' or set environment variabale ZXING_CPP_LIB_DIR.")
	}

	// manual bindings.rs generation:
	// bindgen core/src/ZXingC.h -o src/bindings.rs --no-prepend-enum-name --merge-extern-blocks --use-core --no-doc-comments --no-layout-tests --with-derive-partialeq --allowlist-item "ZXing.*" -- -DZXING_EXPERIMENTAL_API

	Ok(())
}
