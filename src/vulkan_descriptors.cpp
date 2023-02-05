#include "vulkan_descriptors.h"

VkDescriptorSet createSingleImageDecriptorSet(VulkanEngine* vk, VkDescriptorPool pool,VkDescriptorSetLayout layout, VkE_Image image) {
    VulkanBackEndData bd = vk->getBackEndData();

    // Create Descriptor Set:
    VkDescriptorSet descriptor_set;
    {
        VkDescriptorSetAllocateInfo alloc_info = {};
        alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        alloc_info.descriptorPool = pool;
        alloc_info.descriptorSetCount = 1;
        alloc_info.pSetLayouts = &layout;
        if(vkAllocateDescriptorSets(bd.device, &alloc_info, &descriptor_set) != VK_SUCCESS){
            throw std::runtime_error("failed to allocate single image descriptor set!");
        }
    }

    // Update the Descriptor Set:
    {
        VkDescriptorImageInfo desc_image[1] = {};
        desc_image[0].sampler = image.sampler;
        desc_image[0].imageView = image.view;
        desc_image[0].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        VkWriteDescriptorSet write_desc[1] = {};
        write_desc[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write_desc[0].dstSet = descriptor_set;
        write_desc[0].descriptorCount = 1;
        write_desc[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        write_desc[0].pImageInfo = desc_image;
        vkUpdateDescriptorSets(bd.device, 1, write_desc, 0, NULL);
    }
    return descriptor_set;
}