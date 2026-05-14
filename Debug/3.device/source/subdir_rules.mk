################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL = cmd.exe

# Each subdirectory must supply rules for building sources it contributes
3.device/source/%.obj: ../3.device/source/%.c $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: C2000 Compiler'
	"D:/1-ti/ccs/tools/compiler/ti-cgt-c2000_22.6.1.LTS/bin/cl2000" -v28 -ml -mt --cla_support=cla1 --float_support=fpu32 --tmu_support=tmu0 --vcu_support=vcu2 --include_path="D:/1-ti/workspace/28379d_BLDC_six-step" --include_path="D:/1-ti/ccs/tools/compiler/ti-cgt-c2000_22.6.1.LTS/include" --include_path="D:/1-ti/workspace/28379d_BLDC_six-step/3.device/include" --include_path="D:/1-ti/workspace/28379d_BLDC_six-step/4.common/include" --include_path="D:/1-ti/workspace/28379d_BLDC_six-step/5.headers/include" --define=CPU1 --define=_LAUNCHXL_F28379D -g --diag_warning=225 --diag_wrap=off --display_error_number --abi=eabi --preproc_with_compile --preproc_dependency="3.device/source/$(basename $(<F)).d_raw" --obj_directory="3.device/source" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: "$<"'
	@echo ' '


