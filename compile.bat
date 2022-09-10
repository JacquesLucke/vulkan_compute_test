cl .\main.cc "C:\VulkanSDK\1.3.224.1\Lib\vulkan-1.lib" /I"C:\VulkanSDK\1.3.224.1\Include" /EHsc
dxc -T cs_6_0 -spirv -E "Main" -fspv-target-env="vulkan1.1" -fvk-use-dx-layout -Fo "my_shader.spv" ".\my_shader.hlsl"
main.exe
