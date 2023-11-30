INCLUDE_DIR = -Iinclude_linux/ -I/mnt/d/Projekte_C_C++/source/repos2/Arcade-Learning-Environment/src
CPPFLAGS = -std=c++17 -pthread -O2 -g -DUSE_CUDA

TrainingLogger.obj: ./src/TrainingLogger.cpp
	g++ -c ./src/TrainingLogger.cpp  $(INCLUDE_DIR) -o ./OBJs/TrainingLogger.obj $(CPPFLAGS)

TrainingEncoder.obj: ./src/TrainingEncoder.cpp
	g++ -c ./src/TrainingEncoder.cpp  $(INCLUDE_DIR) -o ./OBJs/TrainingEncoder.obj $(CPPFLAGS)

TrainingController.obj: ./src/trainingController/TrainingController.cpp
	g++ -c ./src/trainingController/TrainingController.cpp  $(INCLUDE_DIR) -o ./OBJs/trainingController/TrainingController.obj $(CPPFLAGS)

TrainingControllerContinuous.obj: ./src/trainingController/TrainingControllerContinuous.cpp
	g++ -c ./src/trainingController/TrainingControllerContinuous.cpp  $(INCLUDE_DIR) -o ./OBJs/trainingController/TrainingControllerContinuous.obj $(CPPFLAGS)

TrainingControllerEpisodic.obj: ./src/trainingController/TrainingControllerEpisodic.cpp
	g++ -c ./src/trainingController/TrainingControllerEpisodic.cpp  $(INCLUDE_DIR) -o ./OBJs/trainingController/TrainingControllerEpisodic.obj $(CPPFLAGS)

TrainingRewarder.obj: ./src/TrainingRewarder.cpp
	g++ -c ./src/TrainingRewarder.cpp  $(INCLUDE_DIR) -o ./OBJs/TrainingRewarder.obj $(CPPFLAGS)

TrainingParser.obj: ./src/TrainingParser.cpp
	g++ -c ./src/TrainingParser.cpp  $(INCLUDE_DIR) -o ./OBJs/TrainingParser.obj $(CPPFLAGS)

Main.obj: ./src/Main.cpp
	g++ -c ./src/Main.cpp  $(INCLUDE_DIR) -o ./OBJs/Main.obj $(CPPFLAGS)

Models.obj: ./src/Models.cpp
	g++ -c ./src/Models.cpp  $(INCLUDE_DIR) -o ./OBJs/Models.obj $(CPPFLAGS)

Environment.obj: ./src/Environment.cpp
	g++ -c ./src/Environment.cpp  $(INCLUDE_DIR) -o ./OBJs/Environment.obj $(CPPFLAGS)

Random.obj: ./src/util/Random.cpp
	g++ -c ./src/util/Random.cpp  $(INCLUDE_DIR) -o ./OBJs/util/Random.obj $(CPPFLAGS)

StringUtils.obj: ./src/util/StringUtils.cpp
	g++ -c ./src/util/StringUtils.cpp  $(INCLUDE_DIR) -o ./OBJs/util/StringUtils.obj $(CPPFLAGS)

GZip.obj: ./src/util/compression/GZip.cpp
	g++ -c ./src/util/compression/GZip.cpp  $(INCLUDE_DIR) -o ./OBJs/util/compression/GZip.obj $(CPPFLAGS)

HTTPHelper.obj: ./src/util/HTTPHelper.cpp
	g++ -c ./src/util/HTTPHelper.cpp  $(INCLUDE_DIR) -o ./OBJs/util/HTTPHelper.obj $(CPPFLAGS)

IOUtils.obj: ./src/util/IOUtils.cpp
	g++ -c ./src/util/IOUtils.cpp  $(INCLUDE_DIR) -o ./OBJs/util/IOUtils.obj $(CPPFLAGS)

Serialization.obj: ./src/util/Serialization.cpp
	g++ -c ./src/util/Serialization.cpp  $(INCLUDE_DIR) -o ./OBJs/util/Serialization.obj $(CPPFLAGS)

clean:
	rm -r ./OBJs/

all: TrainingLogger.obj TrainingEncoder.obj TrainingController.obj TrainingControllerContinuous.obj TrainingControllerEpisodic.obj TrainingRewarder.obj TrainingParser.obj Main.obj Models.obj Environment.obj Random.obj StringUtils.obj GZip.obj HTTPHelper.obj IOUtils.obj Serialization.obj
	g++ ./OBJs/TrainingLogger.obj ./OBJs/TrainingEncoder.obj ./OBJs/trainingController/TrainingController.obj ./OBJs/trainingController/TrainingControllerContinuous.obj ./OBJs/trainingController/TrainingControllerEpisodic.obj ./OBJs/TrainingRewarder.obj ./OBJs/TrainingParser.obj ./OBJs/Main.obj ./OBJs/Models.obj ./OBJs/Environment.obj ./OBJs/util/Random.obj ./OBJs/util/StringUtils.obj ./OBJs/util/compression/GZip.obj ./OBJs/util/HTTPHelper.obj ./OBJs/util/IOUtils.obj ./OBJs/util/Serialization.obj -L. -L./lib/torch -l:libz.a -lm -pthread -ldl -lstdc++ -l:libgtest.a -l:libgtest_main.a -l:libtensorpipe.a -l:libtensorpipe_cuda.a -l:libtensorpipe_uv.a -l:libasmjit.a -l:libbenchmark.a -l:libbenchmark_main.a -l:libcaffe2_protos.a -l:libclog.a -l:libdnnl.a -l:libdnnl_graph.a -l:libfbgemm.a -l:libfmt.a -l:libfoxi_loader.a -l:libgloo.a -l:libgloo_cuda.a -l:libgmock.a -l:libgmock_main.a -l:libittnotify.a -l:libkineto.a -l:libnnpack.a -l:libnnpack_reference_layers.a -l:libonnx.a -l:libonnx_proto.a -l:libprotobuf.a -l:libprotobuf-lite.a -l:libprotoc.a  -l:libpytorch_qnnpack.a -l:libqnnpack.a -l:libunbox_lib.a -l:libXNNPACK.a -l:libcpuinfo.a -l:libcpuinfo_internals.a -l:libpthreadpool.a -l:libtorchbind_test.so -l:libtorch_python.so -l:libtorch_global_deps.so -l:libtorch_cuda_linalg.so -l:libtorch_cuda.so -l:libtorch_cpu.so -l:libtorch.so -l:libshm.so -l:libnvfuser_codegen.so -l:libnnapi_backend.so -l:libjitbackend_test.so -l:libcaffe2_nvrtc.so -l:libc10d_cuda_test.so -l:libc10_cuda.so -l:libc10.so -l:libbackend_with_compiler.so -l:libale.a -l:libz.a -shared-libgcc -Wl,-rpath='$$ORIGIN' -o Breakout_PPO.out
