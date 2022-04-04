PACKAGE = com.example.zxingcppdemo

all: debug install start

debug:
	./gradlew assembleDebug

lint:
	./gradlew :zxingcpp:lintDebug

aar: clean lint
	./gradlew :zxingcpp:assembleRelease

install:
	adb $(TARGET) install -r app/build/outputs/apk/debug/app-debug.apk

start:
	adb $(TARGET) shell 'am start -n $(PACKAGE)/.MainActivity'

uninstall:
	adb $(TARGET) uninstall $(PACKAGE)

clean:
	./gradlew clean
	rm -rf zxingcpp/.cxx
