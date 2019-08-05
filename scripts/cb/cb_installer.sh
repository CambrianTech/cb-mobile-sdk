#!/usr/bin/env bash

source cb_common.sh

function cb_installer() {

    cb_logger "Installing $1"
    
    case $1 in 
        android)
            download_dependency cambrian-android
            ;;
        keras)
            install_keras
            ;;
        tensorflow)
            install_tensorflow
            ;;
        
    esac
    cb_logger
    cb_logger "Completed installing $@"
    cb_logger
    return 0
}


function install_keras() {
    download_dependency tensorflow
    download_dependency keras

    cd ${CB}/third-party/keras/src/keras

    ${PYTHON_EXE} setup.py install

    test_keras
}

function test_keras() {
    echo_color ${LightGreen} "Testing keras installation"

    ${PYTHON_EXE} ${CB}/third-party/keras/src/keras/examples/mnist_mlp.py
}

function install_tensorflow() {
    download_dependency tensorflow

    #${PIP_EXE} install tensorflow  

    cd "${CB}/third-party/tensorflow/src/tensorflow"

    if [[ "$unamestr" == 'Darwin' ]]; then
        install_tool "numpy --with-python3"
        install_tool bazel
        PYTHON_PATH="/usr/local/bin/${PYTHON_EXE}"
    else
        install_tool ${PYTHON_EXE}-numpy 
        install_tool ${PYTHON_EXE}-dev 
        install_tool ${PYTHON_EXE}-pip 
        install_tool ${PYTHON_EXE}-wheel
        install_tool bazel
        PYTHON_PATH="/usr/local/bin/${PYTHON_EXE}"
    fi

    ${PIP_EXE} install six numpy wheel 

    bazel clean --expunge

    echo_color ${LightCyan} "\n${sb}Cambrian typical answers to tensorflow installation questions:${eb}"

    echo_color ${White} "Default python located at: ${PYTHON_PATH}"

    echo_color ${White} "No to MKL support"

    echo_color ${White} "No to MPI support"

    echo_color ${White} "Ignore flags"

    echo ""
 
    ./configure

    bazel build --config=opt //tensorflow/tools/pip_package:build_pip_package

    bazel-bin/tensorflow/tools/pip_package/build_pip_package /tmp/tensorflow_pkg

    sudo ${PIP_EXE} install /tmp/tensorflow_pkg/tensorflow-*.whl

    test_tensorflow
}

function test_tensorflow() {
    echo_color ${LightGreen} "Testing tensorflow installation"

    ${PYTHON_EXE} -c "import tensorflow as tf; \
                      hello = tf.constant('Hello, TensorFlow!'); \
                      sess = tf.Session(); \
                      print(sess.run(hello));"
}