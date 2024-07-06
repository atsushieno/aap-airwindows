
ABIS_SIMPLE= x86 x86_64 armeabi-v7a arm64-v8a

all: build-all

build-all: \
	build-aap-core \
	create-local-prop \
	build-java

build-aap-core:
	if [ ! -f external/aap-core/local.properties ] ; then \
		if [ `uname` == "Darwin" ] ; then \
			echo "sdk.dir=$(HOME)/Library/Android/sdk" > external/aap-core/local.properties ; \
		else \
			echo "sdk.dir=$(HOME)/Android/Sdk" > external/aap-core/local.properties ; \
		fi ; \
	fi
	cd external/aap-core && ./gradlew publishToMavenLocal

## Build utility

create-local-prop:
	if [ ! -f local.properties ] ; then \
		if [ `uname` == "Darwin" ] ; then \
			echo "sdk.dir=$(HOME)/Library/Android/sdk" > local.properties ; \
		else \
			echo "sdk.dir=$(HOME)/Android/Sdk" > local.properties ; \
		fi ; \
	fi

build-java: create-local-prop
	ANDROID_SDK_ROOT=$(ANDROID_SDK_ROOT) ./gradlew build
 
