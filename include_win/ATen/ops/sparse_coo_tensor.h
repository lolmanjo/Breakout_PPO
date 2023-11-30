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



#include <ATen/ops/sparse_coo_tensor_ops.h>

namespace at {


// aten::sparse_coo_tensor.size(int[] size, *, ScalarType? dtype=None, Layout? layout=None, Device? device=None, bool? pin_memory=False) -> Tensor
inline at::Tensor sparse_coo_tensor(at::IntArrayRef size, at::TensorOptions options) {
    return at::_ops::sparse_coo_tensor_size::call(size, optTypeMetaToScalarType(options.dtype_opt()), options.layout_opt(), options.device_opt(), options.pinned_memory_opt());
}
// aten::sparse_coo_tensor.size(int[] size, *, ScalarType? dtype=None, Layout? layout=None, Device? device=None, bool? pin_memory=False) -> Tensor
inline at::Tensor sparse_coo_tensor(at::IntArrayRef size, c10::optional<at::ScalarType> dtype, c10::optional<at::Layout> layout, c10::optional<at::Device> device, c10::optional<bool> pin_memory) {
    return at::_ops::sparse_coo_tensor_size::call(size, dtype, layout, device, pin_memory);
}

// aten::sparse_coo_tensor.indices(Tensor indices, Tensor values, *, ScalarType? dtype=None, Layout? layout=None, Device? device=None, bool? pin_memory=None) -> Tensor
inline at::Tensor sparse_coo_tensor(const at::Tensor & indices, const at::Tensor & values, at::TensorOptions options={}) {
    return at::_ops::sparse_coo_tensor_indices::call(indices, values, optTypeMetaToScalarType(options.dtype_opt()), options.layout_opt(), options.device_opt(), options.pinned_memory_opt());
}
// aten::sparse_coo_tensor.indices(Tensor indices, Tensor values, *, ScalarType? dtype=None, Layout? layout=None, Device? device=None, bool? pin_memory=None) -> Tensor
inline at::Tensor sparse_coo_tensor(const at::Tensor & indices, const at::Tensor & values, c10::optional<at::ScalarType> dtype, c10::optional<at::Layout> layout, c10::optional<at::Device> device, c10::optional<bool> pin_memory) {
    return at::_ops::sparse_coo_tensor_indices::call(indices, values, dtype, layout, device, pin_memory);
}

// aten::sparse_coo_tensor.indices_size(Tensor indices, Tensor values, int[] size, *, ScalarType? dtype=None, Layout? layout=None, Device? device=None, bool? pin_memory=None) -> Tensor
inline at::Tensor sparse_coo_tensor(const at::Tensor & indices, const at::Tensor & values, at::IntArrayRef size, at::TensorOptions options={}) {
    return at::_ops::sparse_coo_tensor_indices_size::call(indices, values, size, optTypeMetaToScalarType(options.dtype_opt()), options.layout_opt(), options.device_opt(), options.pinned_memory_opt());
}
// aten::sparse_coo_tensor.indices_size(Tensor indices, Tensor values, int[] size, *, ScalarType? dtype=None, Layout? layout=None, Device? device=None, bool? pin_memory=None) -> Tensor
inline at::Tensor sparse_coo_tensor(const at::Tensor & indices, const at::Tensor & values, at::IntArrayRef size, c10::optional<at::ScalarType> dtype, c10::optional<at::Layout> layout, c10::optional<at::Device> device, c10::optional<bool> pin_memory) {
    return at::_ops::sparse_coo_tensor_indices_size::call(indices, values, size, dtype, layout, device, pin_memory);
}

// aten::sparse_coo_tensor.size_out(int[] size, *, Tensor(a!) out) -> Tensor(a!)
inline at::Tensor & sparse_coo_tensor_out(at::Tensor & out, at::IntArrayRef size) {
    return at::_ops::sparse_coo_tensor_size_out::call(size, out);
}
// aten::sparse_coo_tensor.size_out(int[] size, *, Tensor(a!) out) -> Tensor(a!)
inline at::Tensor & sparse_coo_tensor_outf(at::IntArrayRef size, at::Tensor & out) {
    return at::_ops::sparse_coo_tensor_size_out::call(size, out);
}

}
