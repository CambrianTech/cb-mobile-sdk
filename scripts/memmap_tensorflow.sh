
TF_ROOT=$CB/third-party/tensorflow/src/tensorflow

in_graph="model/output_graph.pb"
out_graph="model/optimized_graph.pb"
latest_checkpoint="model.ckpt-8361242"

input_names="input"
output_names="output"
resolution="320,320"

set -e #exit on error

#first, export to pb:
python export_to_mobile.py --dataset=ade20k --filter-scale=2 --model=others

# pushd $HOME/installs/tensorflow;  bazel build tensorflow/python/tools:freeze_graph; popd 
# $TF_ROOT/bazel-bin/tensorflow/python/tools/freeze_graph \
# 	--input_graph=$in_graph \
# 	--input_checkpoint=$latest_checkpoint \
# 	--output_graph=$out_graph \
# 	--output_node_names=$output_names

pushd $HOME/installs/tensorflow;  bazel build tensorflow/tools/graph_transforms:summarize_graph; popd
$TF_ROOT/bazel-bin/tensorflow/tools/graph_transforms/summarize_graph --in_graph=$in_graph


# Transforms are:
# add_default_attributes
# backport_concatv2
# backport_tensor_array_v3
# flatten_atrous_conv
# fold_batch_norms
# fold_constants
# fold_old_batch_norms
# freeze_requantization_ranges
# fuse_pad_and_conv
# fuse_remote_graph
# fuse_resize_and_conv
# fuse_resize_pad_and_conv
# insert_logging
# merge_duplicate_nodes
# obfuscate_names
# place_remote_graph_arguments
# quantize_nodes
# quantize_weights
# remove_attribute
# remove_control_dependencies
# remove_device
# remove_nodes
# rename_attribute
# rename_op
# rewrite_quantized_stripped_model_for_hexagon
# round_weights
# set_device
# sort_by_execution_order
# sparsify_gather
# strip_unused_nodes

# pushd $HOME/installs/tensorflow; bazel build tensorflow/tools/graph_transforms:transform_graph; popd
$TF_ROOT/bazel-bin/tensorflow/tools/graph_transforms/transform_graph \
	--in_graph=$in_graph \
	--out_graph=$out_graph \
	--inputs=$input_names \
	--outputs=$output_names \
	--transforms='
		add_default_attributes
		strip_unused_nodes(type=float, shape="$resolution,3")
		remove_nodes(op=Identity, op=CheckNumerics)
		fold_constants(ignore_errors=true)
		fold_batch_norms
		fold_old_batch_norms
  		round_weights
  		sort_by_execution_order
		'

# $TF_ROOT/bazel-bin/tensorflow/tools/graph_transforms/summarize_graph --in_graph=$out_graph


pushd $HOME/installs/tensorflow; bazel build tensorflow/python/tools:optimize_for_inference; popd
$TF_ROOT/bazel-bin/tensorflow/python/tools/optimize_for_inference \
	--input=$out_graph \
	--output=$out_graph \
	--input_names=$input_names \
	--output_names=$output_names


pushd $HOME/installs/tensorflow; bazel build tensorflow/python/tools:print_selective_registration_header; popd
$TF_ROOT/bazel-bin/tensorflow/python/tools/print_selective_registration_header \
    --graphs=$out_graph > $out_graph.h

#maybe old stuff:

#last step:
pushd $HOME/installs/tensorflow; bazel build tensorflow/contrib/util:convert_graphdef_memmapped_format; popd
$TF_ROOT/bazel-bin/tensorflow/contrib/util/convert_graphdef_memmapped_format \
	--in_graph=$out_graph \
	--out_graph=$out_graph

#compress:
#extension=xz
#xz --lzma2 -v --threads=0 -5 --check=none --block-size=10000000 $out_graph
#mv /tmp/${model_name}.${extension} ${model_directory_path}

