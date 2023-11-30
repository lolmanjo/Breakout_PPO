CPPFLAGS = -std=c++17 -pthread -O2 -g -DUSE_CUDA

TrainingLogger.obj: ./src/TrainingLogger.cpp
	g++ -c ./src/TrainingLogger.cpp  -Iinclude/ -o ./OBJs/TrainingLogger.obj $(CPPFLAGS)

TrainingEncoder.obj: ./src/TrainingEncoder.cpp
	g++ -c ./src/TrainingEncoder.cpp  -Iinclude/ -o ./OBJs/TrainingEncoder.obj $(CPPFLAGS)

TrainingController.obj: ./src/TrainingController.cpp
	g++ -c ./src/TrainingController.cpp  -Iinclude/ -o ./OBJs/TrainingController.obj $(CPPFLAGS)

TrainingRewarder.obj: ./src/TrainingRewarder.cpp
	g++ -c ./src/TrainingRewarder.cpp  -Iinclude/ -o ./OBJs/TrainingRewarder.obj $(CPPFLAGS)

TrainingParser.obj: ./src/TrainingParser.cpp
	g++ -c ./src/TrainingParser.cpp  -Iinclude/ -o ./OBJs/TrainingParser.obj $(CPPFLAGS)

Main.obj: ./src/Main.cpp
	g++ -c ./src/Main.cpp  -Iinclude/ -o ./OBJs/Main.obj $(CPPFLAGS)

Models.obj: ./src/Models.cpp
	g++ -c ./src/Models.cpp  -Iinclude/ -o ./OBJs/Models.obj $(CPPFLAGS)

Environment.obj: ./src/Environment.cpp
	g++ -c ./src/Environment.cpp  -Iinclude/ -o ./OBJs/Environment.obj $(CPPFLAGS)

Random.obj: ./src/Random.cpp
	g++ -c ./src/Random.cpp  -Iinclude/ -o ./OBJs/Random.obj $(CPPFLAGS)

StringUtils.obj: ./src/StringUtils.cpp
	g++ -c ./src/StringUtils.cpp  -Iinclude/ -o ./OBJs/StringUtils.obj $(CPPFLAGS)

clean:
	rm -r ./OBJs/

all: TrainingLogger.obj TrainingEncoder.obj TrainingController.obj TrainingRewarder.obj TrainingParser.obj Main.obj Models.obj Environment.obj Random.obj StringUtils.obj
	g++ ./OBJs/TrainingLogger.obj ./OBJs/TrainingEncoder.obj ./OBJs/TrainingController.obj ./OBJs/TrainingRewarder.obj ./OBJs/TrainingParser.obj ./OBJs/Main.obj ./OBJs/Models.obj ./OBJs/Environment.obj ./OBJs/Random.obj ./OBJs/StringUtils.obj -L. -L./lib/torch -l:libz.a -lm -pthread -ldl -lstdc++ -l:libgtest.a -l:libgtest_main.a -l:libtensorpipe.a -l:libtensorpipe_cuda.a -l:libtensorpipe_uv.a -l:libasmjit.a -l:libbenchmark.a -l:libbenchmark_main.a -l:libcaffe2_protos.a -l:libclog.a -l:libdnnl.a -l:libdnnl_graph.a -l:libfbgemm.a -l:libfmt.a -l:libfoxi_loader.a -l:libgloo.a -l:libgloo_cuda.a -l:libgmock.a -l:libgmock_main.a -l:libittnotify.a -l:libkineto.a -l:libnnpack.a -l:libnnpack_reference_layers.a -l:libonnx.a -l:libonnx_proto.a -l:libprotobuf.a -l:libprotobuf-lite.a -l:libprotoc.a  -l:libpytorch_qnnpack.a -l:libqnnpack.a -l:libunbox_lib.a -l:libXNNPACK.a -l:libcpuinfo.a -l:libcpuinfo_internals.a -l:libpthreadpool.a -l:libtorchbind_test.so -l:libtorch_python.so -l:libtorch_global_deps.so -l:libtorch_cuda_linalg.so -l:libtorch_cuda.so -l:libtorch_cpu.so -l:libtorch.so -l:libshm.so -l:libnvfuser_codegen.so -l:libnnapi_backend.so -l:libjitbackend_test.so -l:libcaffe2_nvrtc.so -l:libc10d_cuda_test.so -l:libc10_cuda.so -l:libc10.so -l:libbackend_with_compiler.so -shared-libgcc -Wl,-rpath='$$ORIGIN' -o Breakout_PPO.out
