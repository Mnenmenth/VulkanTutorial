/**
  * Created by Earl Kennedy
  * https://github.com/Mnenmenth
  */

#ifndef VULKANTUTORIAL_UBO_H
#define VULKANTUTORIAL_UBO_H

#include <glm/glm.hpp>

namespace UBO
{
    struct MVP
    {
        glm::mat4 model;
        glm::mat4 view;
        glm::mat4 proj;
    };
}

#endif //VULKANTUTORIAL_UBO_H
