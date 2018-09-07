build:
	mkdir -p build
	$(eval MOSQ_PATH := $(shell brew --prefix mosquitto))
	g++ src/*.cpp -o build/sampleapp -lmosquittopp -I$(MOSQ_PATH)/include -L$(MOSQ_PATH)/lib

clean:
	rm -rf build

.PHONY: build
