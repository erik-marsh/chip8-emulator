#pragma once

#include <string>

#include <glad\glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

    class Shader
    {
    public: // TODO: change back later
        unsigned int m_programID;

        std::string readFile(const char* filePath) const;
        unsigned int compileShader(const char* src, GLenum type) const;
        void linkProgram(unsigned int vertexShader, unsigned int fragmentShader) const;

    public:
        Shader(const char* vertexShaderPath, const char* fragmentShaderPath);

        void use();

        //uniform functions
        bool uniformExists(const char* name);

        void setFloat(const char* name, float x);
        void setFloat(const char* name, float x, float y);
        void setFloat(const char* name, float x, float y, float z);
        void setFloat(const char* name, float x, float y, float z, float w);

        void setInt(const char* name, int x);
        void setInt(const char* name, int x, int y);
        void setInt(const char* name, int x, int y, int z);
        void setInt(const char* name, int x, int y, int z, int w);

        void setBool(const char* name, bool x);
        void setBool(const char* name, bool x, bool y);
        void setBool(const char* name, bool x, bool y, bool z);
        void setBool(const char* name, bool x, bool y, bool z, bool w);

        void setMat4(const char* name, const glm::mat4& matrix);
    };