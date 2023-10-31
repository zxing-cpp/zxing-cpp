Pod::Spec.new do |s|
  s.name = 'zxing-cpp'
  s.version = '2.1.0'
  s.summary = 'C++ port of ZXing'
  s.homepage = 'https://github.com/zxing-cpp/zxing-cpp'
  s.author = 'axxel'
  s.readme = 'https://raw.githubusercontent.com/zxing-cpp/zxing-cpp/master/wrappers/ios/README.md'
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
  s.library = ['c++']
  s.pod_target_xcconfig = {
    'CLANG_CXX_LANGUAGE_STANDARD' => 'c++20'
  }

  s.default_subspec = 'Wrapper'

  s.subspec 'Core' do |ss|
    ss.source_files = 'core/src/**/*.{h,c,cpp}'
    ss.private_header_files = 'core/src/**/*.h'
  end

  s.subspec 'Wrapper' do |ss|
    ss.dependency 'zxing-cpp/Core'
    ss.frameworks = 'CoreGraphics', 'CoreImage', 'CoreVideo'
    ss.source_files = 'wrappers/ios/Sources/Wrapper/**/*.{h,m,mm}'
    ss.public_header_files = 'wrappers/ios/Sources/Wrapper/Reader/{ZXIBarcodeReader,ZXIResult,ZXIPosition,ZXIPoint,ZXIGTIN,ZXIDecodeHints}.h',
                             'wrappers/ios/Sources/Wrapper/Writer/{ZXIBarcodeWriter,ZXIEncodeHints}.h',
                             'wrappers/ios/Sources/Wrapper/{ZXIErrors,ZXIFormat}.h'
    ss.exclude_files = 'wrappers/ios/Sources/Wrapper/UmbrellaHeader.h'
  end
end
