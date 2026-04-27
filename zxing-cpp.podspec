Pod::Spec.new do |s|
  s.name = 'zxing-cpp'
  s.version = '3.0.2'
  s.summary = 'C++ port of ZXing'
  s.homepage = 'https://github.com/zxing-cpp/zxing-cpp'
  s.author = 'axxel'
  s.readme = 'https://raw.githubusercontent.com/zxing-cpp/zxing-cpp/master/wrappers/swift/README.md'
  s.license = {
    :type => 'Apache License 2.0',
    :file => 'LICENSE'
  }
  if ENV['ZXING_POD_LOCAL'] == '1'
    local_path = File.expand_path(__dir__)
    s.source = {
      :git => "file://#{local_path}",
      :branch => 'swift-wrapper',
      :submodules => true
    }
  else
    s.source = {
      :git => 'https://github.com/axxel/zxing-cpp.git',
      :branch => 'swift-wrapper',
      :submodules => true
    }
  end
  s.module_name = 'ZXingCpp'
  s.platform = :ios, '13.0'
  s.swift_versions = '5.0'
  s.compiler_flags = [
    '-DZXING_INTERNAL',
    '-Wno-comma',
    '-Wno-shorten-64-to-32',
  ]
  s.pod_target_xcconfig = {
    'CLANG_CXX_LANGUAGE_STANDARD' => 'c++20',
  }
  s.prepare_command = <<-CMD
    set -e
    # Materialize libzint symlinks
    find core/src/libzint -type l | while read symlink; do
      target=$(readlink -f "$symlink")
      rm -f "$symlink"
      cp "$target" "$symlink"
    done

    # Remove import statement since C bridge is in the same module for CocoaPods
    find wrappers/swift/Sources/ZXingCpp -name "*.swift" -type f -exec sed -i '' '/^import ZXingCBridge$/d' {} \\;
  CMD

  s.default_subspec = 'Swift'

  # Swift wrapper subspec (Core + CBridge + Swift code)
  s.subspec 'Swift' do |ss|
    ss.source_files = 'core/src/**/*.{h,c,cpp}', 'wrappers/swift/Sources/ZXingCBridge/**/*.{c,h}', 'wrappers/swift/Sources/ZXingCpp/**/*.swift'
    ss.exclude_files = [
      'core/src/*/*{Writer,Encoder}*',
      'core/src/{MultiFormatWriter,ReedSolomonEncoder}.*',
    ]
    ss.public_header_files = 'wrappers/swift/Sources/ZXingCBridge/bundled/*.h'
  end

  s.subspec 'LegacyCore' do |ss|
    ss.source_files = 'core/src/**/*.{h,c,cpp}', 'wrappers/ios/Sources/Wrapper/Version.h'
    ss.exclude_files = [ 'core/src/libzint/**' ]
    ss.private_header_files = 'core/src/**/*.h'
  end

  s.subspec 'LegacyWrapper' do |ss|
    ss.dependency 'zxing-cpp/LegacyCore'
    ss.frameworks = 'CoreGraphics', 'CoreImage', 'CoreVideo'
    ss.source_files = 'wrappers/ios/Sources/Wrapper/**/*.{h,m,mm}'
    ss.public_header_files = 'wrappers/ios/Sources/Wrapper/Reader/{ZXIBarcodeReader,ZXIResult,ZXIPosition,ZXIPoint,ZXIGTIN,ZXIReaderOptions}.h',
                             'wrappers/ios/Sources/Wrapper/Writer/{ZXIBarcodeWriter,ZXIWriterOptions}.h',
                             'wrappers/ios/Sources/Wrapper/{ZXIErrors,ZXIFormat,Version}.h'
    ss.exclude_files = 'wrappers/ios/Sources/Wrapper/UmbrellaHeader.h'
  end
end
