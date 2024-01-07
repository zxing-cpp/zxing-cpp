use miette::IntoDiagnostic;
use std::env;
use std::path::PathBuf;

fn main() -> miette::Result<()> {
    if let Ok(lib_dir) = env::var("ZXING_CPP_LIB_DIR") {
        println!("cargo:rustc-link-search=native={}", lib_dir);
        println!("cargo:rustc-link-lib=static=ZXing");
    }

    let src_path: PathBuf = env::var("ZXING_CPP_SRC_DIR")
        .into_diagnostic()
        .map_err(|e| e.context("ZXING_CPP_SRC_DIR"))?
        .into();

    let ext_path = std::path::PathBuf::from("src/extensions");
    let mut b = autocxx_build::Builder::new("src/bindings.rs", [&src_path, &ext_path])
        .extra_clang_args(&["-std=c++17", "-Wc++17-extensions"])
        .build()?;

    b.cpp(true)
        .std("c++17")
        .flag_if_supported("-Wc++17-extensions")
        .compile("zxing-cpp2rs");

    println!("cargo:rerun-if-changed=src/bindings.rs");
    println!("cargo:rerun-if-changed=src/extensions");

    Ok(())
}
