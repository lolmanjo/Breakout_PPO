#pragma once

// @generated by torchgen/gen.py from Function.h

#include <ATen/Context.h>
#include <ATen/DeviceGuard.h>
#include <ATen/TensorUtils.h>
#include <ATen/TracerMode.h>
#include <ATen/core/Generator.h>
#include <ATen/core/Reduction.h>
#include <ATen/core/Tensor.h>
#include <c10/core/Scalar.h>
#include <c10/core/Storage.h>
#include <c10/core/TensorOptions.h>
#include <c10/util/Deprecated.h>
#include <c10/util/Optional.h>



#include <ATen/ops/cummaxmin_backward_ops.h>

namespace at {


// aten::cummaxmin_backward(Tensor grad, Tensor input, Tensor indices, int dim) -> Tensor
inline at::Tensor cummaxmin_backward(const at::Tensor & grad, const at::Tensor & input, const at::Tensor & indices, int64_t dim) {
    return at::_ops::cummaxmin_backward::call(grad, input, indices, dim);
}

}