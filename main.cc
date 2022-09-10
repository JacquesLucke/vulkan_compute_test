#include <vulkan/vulkan.h>
#include <iostream>
#include <vector>
#include <fstream>
#include <array>

int main(int argc, char const *argv[])
{
    VkApplicationInfo application_info{};
    application_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    application_info.pApplicationName = "Vulkan Test";
    application_info.applicationVersion = VK_MAKE_API_VERSION(0, 1, 0, 0);
    application_info.apiVersion = VK_API_VERSION_1_0;

    VkInstanceCreateInfo instance_create_info{};
    instance_create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instance_create_info.pApplicationInfo = &application_info;

    VkInstance instance;
    vkCreateInstance(&instance_create_info, nullptr, &instance);

    uint32_t device_count = 0;
    vkEnumeratePhysicalDevices(instance, &device_count, nullptr);
    std::cout << "Device Count: " << device_count << "\n";

    std::vector<VkPhysicalDevice> physical_devices(device_count);
    vkEnumeratePhysicalDevices(instance, &device_count, physical_devices.data());
    VkPhysicalDevice physical_device = physical_devices[0];

    VkPhysicalDeviceProperties device_properties{};
    vkGetPhysicalDeviceProperties(physical_devices[0], &device_properties);

    std::cout << device_properties.deviceName << "\n";

    uint32_t queue_family_count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_count, nullptr);
    std::cout << "Queue family count: " << queue_family_count << "\n";

    std::vector<VkQueueFamilyProperties> queue_families(queue_family_count);
    vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_count, queue_families.data());

    for (const VkQueueFamilyProperties &props : queue_families)
    {
        std::cout << &props << " Count: " << props.queueCount << "\n";
        if (props.queueFlags & VK_QUEUE_GRAPHICS_BIT)
        {
            std::cout << "  graphics\n";
        }
        if (props.queueFlags & VK_QUEUE_COMPUTE_BIT)
        {
            std::cout << "  compute\n";
        }
        if (props.queueFlags & VK_QUEUE_TRANSFER_BIT)
        {
            std::cout << "  transfer\n";
        }
        if (props.queueFlags & VK_QUEUE_SPARSE_BINDING_BIT)
        {
            std::cout << "  sparse binding\n";
        }
        if (props.queueFlags & VK_QUEUE_PROTECTED_BIT)
        {
            std::cout << "  protected\n";
        }
    }
    const uint32_t compute_queue_family_index = 0;

    VkDeviceQueueCreateInfo queue_create_info{};
    queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queue_create_info.queueFamilyIndex = compute_queue_family_index;
    queue_create_info.queueCount = 1;
    float queue_priority = 1.0f;
    queue_create_info.pQueuePriorities = &queue_priority;

    VkPhysicalDeviceFeatures device_features{};

    VkDeviceCreateInfo device_create_info{};
    device_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    device_create_info.pQueueCreateInfos = &queue_create_info;
    device_create_info.queueCreateInfoCount = 1;
    device_create_info.pEnabledFeatures = &device_features;

    VkDevice device;
    vkCreateDevice(physical_device, &device_create_info, nullptr, &device);

    VkQueue compute_queue;
    vkGetDeviceQueue(device, compute_queue_family_index, 0, &compute_queue);

    const int num_elements = 10;
    const int buffer_size = num_elements * sizeof(int);

    VkBufferCreateInfo buffer_create_info{};
    buffer_create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    buffer_create_info.size = buffer_size;
    buffer_create_info.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
    buffer_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    buffer_create_info.queueFamilyIndexCount = 1;
    buffer_create_info.pQueueFamilyIndices = &compute_queue_family_index;

    VkBuffer in_buffer;
    vkCreateBuffer(device, &buffer_create_info, nullptr, &in_buffer);
    VkBuffer out_buffer;
    vkCreateBuffer(device, &buffer_create_info, nullptr, &out_buffer);

    VkMemoryRequirements in_buffer_memory_requirements;
    vkGetBufferMemoryRequirements(device, in_buffer, &in_buffer_memory_requirements);
    VkMemoryRequirements out_buffer_memory_requirements;
    vkGetBufferMemoryRequirements(device, out_buffer, &out_buffer_memory_requirements);

    VkPhysicalDeviceMemoryProperties physical_device_memory_properties;
    vkGetPhysicalDeviceMemoryProperties(physical_device, &physical_device_memory_properties);

    uint32_t memory_type_index = -1;
    VkDeviceSize memory_heap_size = -1;
    for (uint32_t current_memory_type_index = 0;
         current_memory_type_index < physical_device_memory_properties.memoryTypeCount;
         current_memory_type_index++)
    {
        VkMemoryType memory_type = physical_device_memory_properties.memoryTypes[current_memory_type_index];
        if (memory_type.propertyFlags & (VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT))
        {
            memory_heap_size = physical_device_memory_properties.memoryHeaps[current_memory_type_index].size;
            memory_type_index = current_memory_type_index;
            break;
        }
    }
    std::cout << "Memory type index: " << memory_type_index << "\n";
    std::cout << "Memory heap size: " << memory_heap_size << "\n";

    VkMemoryAllocateInfo in_buffer_memory_allocate_info{};
    in_buffer_memory_allocate_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    in_buffer_memory_allocate_info.allocationSize = in_buffer_memory_requirements.size;
    in_buffer_memory_allocate_info.memoryTypeIndex = memory_type_index;
    VkDeviceMemory in_device_memory;
    vkAllocateMemory(device, &in_buffer_memory_allocate_info, nullptr, &in_device_memory);

    VkMemoryAllocateInfo out_buffer_memory_allocate_info{};
    out_buffer_memory_allocate_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    out_buffer_memory_allocate_info.allocationSize = out_buffer_memory_requirements.size;
    out_buffer_memory_allocate_info.memoryTypeIndex = memory_type_index;
    VkDeviceMemory out_device_memory;
    vkAllocateMemory(device, &out_buffer_memory_allocate_info, nullptr, &out_device_memory);

    int *in_buffer_ptr;
    vkMapMemory(device, in_device_memory, 0, buffer_size, 0, reinterpret_cast<void **>(&in_buffer_ptr));
    for (int i = 0; i < num_elements; i++)
    {
        in_buffer_ptr[i] = i * i;
    }
    vkUnmapMemory(device, in_device_memory);

    vkBindBufferMemory(device, in_buffer, in_device_memory, 0);
    vkBindBufferMemory(device, out_buffer, out_device_memory, 0);

    std::vector<char> spirv_bytes;
    {
        std::ifstream file{"my_shader.spv", std::ios::binary | std::ios::ate};
        const size_t size = file.tellg();
        file.seekg(0);
        spirv_bytes.resize(size, '\0');
        file.read(spirv_bytes.data(), size);
        std::cout << "Shader file size: " << size << "\n";
    }

    VkShaderModuleCreateInfo shader_module_create_info{};
    shader_module_create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    shader_module_create_info.flags = 0;
    shader_module_create_info.codeSize = spirv_bytes.size();
    shader_module_create_info.pCode = reinterpret_cast<uint32_t *>(spirv_bytes.data());

    VkShaderModule shader_module;
    vkCreateShaderModule(device, &shader_module_create_info, nullptr, &shader_module);

    std::array<VkDescriptorSetLayoutBinding, 2> descriptor_set_layout_bindings{};

    descriptor_set_layout_bindings[0].binding = 0;
    descriptor_set_layout_bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    descriptor_set_layout_bindings[0].descriptorCount = 1;
    descriptor_set_layout_bindings[0].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

    descriptor_set_layout_bindings[1].binding = 1;
    descriptor_set_layout_bindings[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    descriptor_set_layout_bindings[1].descriptorCount = 1;
    descriptor_set_layout_bindings[1].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

    VkDescriptorSetLayoutCreateInfo descriptor_set_layout_create_info{};
    descriptor_set_layout_create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    descriptor_set_layout_create_info.flags = 0;
    descriptor_set_layout_create_info.bindingCount = 2;
    descriptor_set_layout_create_info.pBindings = descriptor_set_layout_bindings.data();

    VkDescriptorSetLayout descriptor_set_layout;
    vkCreateDescriptorSetLayout(device, &descriptor_set_layout_create_info, nullptr, &descriptor_set_layout);

    VkPipelineLayoutCreateInfo pipeline_layout_create_info{};
    pipeline_layout_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipeline_layout_create_info.flags = 0;
    pipeline_layout_create_info.setLayoutCount = 1;
    pipeline_layout_create_info.pSetLayouts = &descriptor_set_layout;

    VkPipelineLayout pipeline_layout;
    vkCreatePipelineLayout(device, &pipeline_layout_create_info, nullptr, &pipeline_layout);

    VkPipelineCacheCreateInfo pipeline_cache_create_info{};
    pipeline_cache_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;

    VkPipelineCache pipeline_cache;
    vkCreatePipelineCache(device, &pipeline_cache_create_info, nullptr, &pipeline_cache);

    VkPipelineShaderStageCreateInfo pipeline_shader_stage_create_info{};
    pipeline_shader_stage_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    pipeline_shader_stage_create_info.flags = 0;
    pipeline_shader_stage_create_info.stage = VK_SHADER_STAGE_COMPUTE_BIT;
    pipeline_shader_stage_create_info.module = shader_module;
    pipeline_shader_stage_create_info.pName = "Main";

    VkComputePipelineCreateInfo compute_pipeline_create_info{};
    compute_pipeline_create_info.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
    compute_pipeline_create_info.flags = 0;
    compute_pipeline_create_info.stage = pipeline_shader_stage_create_info;
    compute_pipeline_create_info.layout = pipeline_layout;

    VkPipeline compute_pipeline;
    vkCreateComputePipelines(device, pipeline_cache, 1, &compute_pipeline_create_info, nullptr, &compute_pipeline);

    VkDescriptorPoolSize descriptor_pool_size{};
    descriptor_pool_size.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    descriptor_pool_size.descriptorCount = 2;

    VkDescriptorPoolCreateInfo descriptor_pool_create_info{};
    descriptor_pool_create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    descriptor_pool_create_info.flags = 0;
    descriptor_pool_create_info.maxSets = 1;
    descriptor_pool_create_info.poolSizeCount = 1;
    descriptor_pool_create_info.pPoolSizes = &descriptor_pool_size;

    VkDescriptorPool descriptor_pool;
    vkCreateDescriptorPool(device, &descriptor_pool_create_info, nullptr, &descriptor_pool);

    VkDescriptorSetAllocateInfo descriptor_set_allocate_info{};
    descriptor_set_allocate_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    descriptor_set_allocate_info.descriptorPool = descriptor_pool;
    descriptor_set_allocate_info.descriptorSetCount = 1;
    descriptor_set_allocate_info.pSetLayouts = &descriptor_set_layout;

    VkDescriptorSet descriptor_set;
    vkAllocateDescriptorSets(device, &descriptor_set_allocate_info, &descriptor_set);

    VkDescriptorBufferInfo in_descriptor_buffer_info{};
    in_descriptor_buffer_info.buffer = in_buffer;
    in_descriptor_buffer_info.offset = 0;
    in_descriptor_buffer_info.range = buffer_size;

    VkDescriptorBufferInfo out_descriptor_buffer_info{};
    out_descriptor_buffer_info.buffer = out_buffer;
    out_descriptor_buffer_info.offset = 0;
    out_descriptor_buffer_info.range = buffer_size;

    std::array<VkWriteDescriptorSet, 2> write_descriptor_sets{};

    write_descriptor_sets[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write_descriptor_sets[0].dstSet = descriptor_set;
    write_descriptor_sets[0].dstBinding = 0;
    write_descriptor_sets[0].dstArrayElement = 0;
    write_descriptor_sets[0].descriptorCount = 1;
    write_descriptor_sets[0].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    write_descriptor_sets[0].pBufferInfo = &in_descriptor_buffer_info;

    write_descriptor_sets[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write_descriptor_sets[1].dstSet = descriptor_set;
    write_descriptor_sets[1].dstBinding = 1;
    write_descriptor_sets[1].dstArrayElement = 0;
    write_descriptor_sets[1].descriptorCount = 1;
    write_descriptor_sets[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    write_descriptor_sets[1].pBufferInfo = &out_descriptor_buffer_info;

    vkUpdateDescriptorSets(device, 2, write_descriptor_sets.data(), 0, nullptr);

    VkCommandPoolCreateInfo command_pool_create_info{};
    command_pool_create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    command_pool_create_info.flags = 0;
    command_pool_create_info.queueFamilyIndex = compute_queue_family_index;

    VkCommandPool command_pool;
    vkCreateCommandPool(device, &command_pool_create_info, nullptr, &command_pool);

    VkCommandBufferAllocateInfo command_buffer_allocate_info{};
    command_buffer_allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    command_buffer_allocate_info.commandPool = command_pool;
    command_buffer_allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    command_buffer_allocate_info.commandBufferCount = 1;

    VkCommandBuffer command_buffer;
    vkAllocateCommandBuffers(device, &command_buffer_allocate_info, &command_buffer);

    VkCommandBufferBeginInfo command_buffer_begin_info{};
    command_buffer_begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    command_buffer_begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(command_buffer, &command_buffer_begin_info);
    vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, compute_pipeline);
    vkCmdBindDescriptorSets(
        command_buffer,
        VK_PIPELINE_BIND_POINT_COMPUTE,
        pipeline_layout,
        0,
        1,
        &descriptor_set,
        0,
        nullptr);
    vkCmdDispatch(command_buffer, num_elements, 1, 1);
    vkEndCommandBuffer(command_buffer);

    VkFenceCreateInfo fence_create_info{};
    fence_create_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;

    VkFence fence;
    vkCreateFence(device, &fence_create_info, nullptr, &fence);

    VkSubmitInfo submit_info{};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &command_buffer;

    vkQueueSubmit(compute_queue, 1, &submit_info, fence);

    vkWaitForFences(device, 1, &fence, true, -1);

    int *out_buffer_ptr;
    vkMapMemory(device, out_device_memory, 0, buffer_size, 0, reinterpret_cast<void **>(&out_buffer_ptr));
    for (int i = 0; i < num_elements; i++) {
        std::cout << i << " " << out_buffer_ptr[i] << "\n";
    }
    vkUnmapMemory(device, out_device_memory);

    std::cout << "Done.\n";
    return 0; 
}
