#pragma once

#include "renderer/common.h"

namespace estun
{
    template <class T>
    struct PushConstant 
    {
        VkShaderStageFlags stageFlags_;

        PushConstant(VkShaderStageFlags stageFlags) : stageFlags_(stageFlags) {}

        uint32_t GetSize() 
		{
            return sizeof(T);
        }

        void SetConst(T &constant)
        {
            constant_ = constant;
        }
        
        T* GetConst()
        {
            return &constant_;
        }
    private:
        T constant_ = {};
    };

}