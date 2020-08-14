#include <glad/glad.h>

#include <iostream>
#include <fstream>
#include <string>
#include <string.h>
#include <sstream>

#include "Shader.h"

    Shader::Shader(const char* vertexShaderPath, const char* fragmentShaderPath)
    {
        //no idea why this happens but you need to explicitly make a new string for the second program's source code
        //could be related to the program string not being correctly null-terminated
        //for the time being i will assume it's an OpenGL issue

        std::string vertexShaderSrc = readFile(vertexShaderPath);
        std::string fragmentShaderSrc = readFile(fragmentShaderPath);

        unsigned int vertexShader = compileShader(vertexShaderSrc.c_str(), GL_VERTEX_SHADER);
        unsigned int fragmentShader = compileShader(fragmentShaderSrc.c_str(), GL_FRAGMENT_SHADER);

        m_programID = glCreateProgram();
        linkProgram(vertexShader, fragmentShader);

        //cleanup
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
    }

    // returns a string with the contents of a file gathered from filePath
    std::string Shader::readFile(const char* filePath) const
    {
        std::ifstream input(filePath);

        std::stringstream sstream;
        std::string result;

        sstream << input.rdbuf();
        result = sstream.str();
        input.close();
        return result;
    }

    unsigned int Shader::compileShader(const char* src, GLenum type) const
    {
        int success;
        char log[512];
        unsigned int shader = glCreateShader(type);
        glShaderSource(shader, 1, &src, NULL); //args: shader, number of source code strings, source code, length of source (can be NULL)
        glCompileShader(shader);

        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            glGetShaderInfoLog(shader, 512, NULL, log);

            std::cerr << "Shader error: ";
            switch (type)
            {
            case GL_VERTEX_SHADER:
                std::cerr << "Vertex ";
                break;
            case GL_GEOMETRY_SHADER:
                std::cerr << "Geometry ";
                break;
            case GL_FRAGMENT_SHADER:
                std::cerr << "Fragment ";
                break;
            }
            std::cerr << "shader failed to compile.\n" << log << std::endl;
        }

        return shader;
    }

    void Shader::linkProgram(unsigned int vertexShader, unsigned int fragmentShader) const
    {
        int success;
        char log[512];

        glAttachShader(m_programID, vertexShader);
        glAttachShader(m_programID, fragmentShader);
        glLinkProgram(m_programID);

        glGetProgramiv(m_programID, GL_LINK_STATUS, &success);
        if (!success)
        {
            glGetProgramInfoLog(m_programID, 512, NULL, log);
            std::cerr << "Shader error: Shader program failed to link.\n" << log << std::endl;
        }
    }

    void Shader::use()
    {
        glUseProgram(m_programID);
    }

    bool Shader::uniformExists(const char* name)
    {
        return (glGetUniformLocation(m_programID, name) == -1) ? false : true;
    }

    void Shader::setFloat(const char* name, float x)
    {
        glUniform1f(glGetUniformLocation(m_programID, name), x);
    }

    void Shader::setFloat(const char* name, float x, float y)
    {
        glUniform2f(glGetUniformLocation(m_programID, name), x, y);
    }

    void Shader::setFloat(const char* name, float x, float y, float z)
    {
        glUniform3f(glGetUniformLocation(m_programID, name), x, y, z);
    }

    void Shader::setFloat(const char* name, float x, float y, float z, float w)
    {
        glUniform4f(glGetUniformLocation(m_programID, name), x, y, z, w);
    }


    void Shader::setInt(const char* name, int x)
    {
        glUniform1i(glGetUniformLocation(m_programID, name), x);
    }

    void Shader::setInt(const char* name, int x, int y)
    {
        glUniform2i(glGetUniformLocation(m_programID, name), x, y);
    }

    void Shader::setInt(const char* name, int x, int y, int z)
    {
        glUniform3i(glGetUniformLocation(m_programID, name), x, y, z);
    }

    void Shader::setInt(const char* name, int x, int y, int z, int w)
    {
        glUniform4i(glGetUniformLocation(m_programID, name), x, y, z, w);
    }


    void Shader::setBool(const char* name, bool x)
    {
        glUniform1i(glGetUniformLocation(m_programID, name), (int)x);
    }

    void Shader::setBool(const char* name, bool x, bool y)
    {
        glUniform2i(glGetUniformLocation(m_programID, name), (int)x, (int)y);
    }

    void Shader::setBool(const char* name, bool x, bool y, bool z)
    {
        glUniform3i(glGetUniformLocation(m_programID, name), (int)x, (int)y, (int)z);
    }

    void Shader::setBool(const char* name, bool x, bool y, bool z, bool w)
    {
        glUniform4i(glGetUniformLocation(m_programID, name), (int)x, (int)y, (int)z, (int)w);
    }

    void Shader::setMat4(const char* name, const glm::mat4& matrix)
    {
        glUniformMatrix4fv(glGetUniformLocation(m_programID, name), 1, GL_FALSE, glm::value_ptr(matrix));
    }