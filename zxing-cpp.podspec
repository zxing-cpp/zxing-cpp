Pod::Spec.new do |s|
  s.name = 'zxing-cpp'
  s.version = '2.1.0'
  s.summary = 'C++ port of ZXing'
  s.homepage = 'https://github.com/zxing-cpp/zxing-cpp'
  s.author = 'axxel'
  s.license = {
    :type => 'Apache License 2.0',
    :file => 'LICENSE'
  }
  s.source = {
    :git => 'https://github.com/zxing-cpp/zxing-cpp.git',
    :tag => "v#{s.version}"
  }
  s.module_name = 'ZXingCpp'
  s.platform = :ios, '11.0'
  s.frameworks = 'CoreGraphics', 'CoreImage', 'CoreVideo'
  s.library = ['c++']
  s.pod_target_xcconfig = {
    'CLANG_CXX_LANGUAGE_STANDARD' => 'c++20'
  }
  s.source_files = 'core/src/**/*.{h,c,cpp}',
                   'wrappers/ios/Sources/Wrapper/**/*.{h,m,mm}'
  s.public_header_files = 'wrappers/ios/Sources/Wrapper/Reader/{ZXIBarcodeReader,ZXIResult,ZXIPosition,ZXIPoint,ZXIDecodeHints}.h',
                          'wrappers/ios/Sources/Wrapper/Writer/ZXIBarcodeWriter.h',
                          'wrappers/ios/Sources/Wrapper/{ZXIErrors,ZXIFormat}.h'
  s.private_header_files = 'core/src/**/*.h'
  s.exclude_files = 'wrappers/ios/Sources/Wrapper/UmbrellaHeader.h'
end
