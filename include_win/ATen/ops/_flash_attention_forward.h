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



#include <ATen/ops/_flash_attention_forward_ops.h>

namespace at {


// aten::_flash_attention_forward(Tensor query, Tensor key, Tensor value, Tensor cum_seq_q, Tensor cum_seq_k, int max_q, int max_k, float dropout_p, bool is_causal, bool return_debug_mask) -> (Tensor output, Tensor softmax_logsumexp, int philox_seed, int philox_offset, Tensor debug_attn_mask)
inline ::std::tuple<at::Tensor,at::Tensor,int64_t,int64_t,at::Tensor> _flash_attention_forward(const at::Tensor & query, const at::Tensor & key, const at::Tensor & value, const at::Tensor & cum_seq_q, const at::Tensor & cum_seq_k, int64_t max_q, int64_t max_k, double dropout_p, bool is_causal, bool return_debug_mask) {
    return at::_ops::_flash_attention_forward::call(query, key, value, cum_seq_q, cum_seq_k, max_q, max_k, dropout_p, is_causal, return_debug_mask);
}

}
