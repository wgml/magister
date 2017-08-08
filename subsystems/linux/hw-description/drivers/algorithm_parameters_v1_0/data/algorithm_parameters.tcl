

proc generate {drv_handle} {
	xdefine_include_file $drv_handle "xparameters.h" "algorithm_parameters" "NUM_INSTANCES" "DEVICE_ID"  "C_ctl_BASEADDR" "C_ctl_HIGHADDR"
}
