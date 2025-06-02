/*
 *  This file is part of OpenGL-FinalProject
 *
 *  Developed by Andrés Ragot - github.com/andresragot
 *
 *  MIT License
 *
 *  Copyright (c) 2025 Andrés Ragot
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a copy
 *  of this software and associated documentation files (the "Software"), to deal
 *  in the Software without restriction, including without limitation the rights
 *  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 *  copies of the Software, and to permit persons to whom the Software is
 *  furnished to do so, subject to the following conditions:
 *
 *  The above copyright notice and this permission notice shall be included in all
 *  copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 *  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 *  SOFTWARE.a
 */

#pragma once

#include <glad/glad.h>

#include <string>
#include <vector>

namespace Ragot
{
    using namespace std;

    /**
     * @brief Class for managing an OpenGL shader.
     */
    class Shader
    {
    private:
        GLuint id; ///< Shader ID.
        string error; ///< Compilation error message.
        bool compilation_succeeded; ///< Flag indicating if compilation succeeded.
        
    protected:
        /**
         * @brief Constructor for the Shader class.
         * @param source_code Vector of shader source code.
         * @param type Shader type (e.g., GL_VERTEX_SHADER, GL_FRAGMENT_SHADER).
         */
        Shader(const vector<string>& source_code, GLenum type);

        /**
         * @brief Compiles the shader.
         * @return Shader ID.
         */
        GLuint compile_shader();

        /**
         * @brief Displays compilation errors.
         */
        void show_compilation_error();

    public:
        Shader() = delete; ///< Deleted default constructor.

        /**
         * @brief Destructor for the Shader class.
         */
        ~Shader()
        {
            glDeleteShader(id);
        }

        /**
         * @brief Gets the shader ID.
         * @return Shader ID.
         */
        GLuint get_id() const
        {
            return id;
        }

        /**
         * @brief Gets the compilation error message.
         * @return Pointer to the error message string.
         */
        string* get_error()
        {
            return error.empty() ? nullptr : &error;
        }

        /**
         * @brief Checks if the shader is compiled successfully.
         * @return True if compilation succeeded, false otherwise.
         */
        bool is_ok() const
        {
            return compilation_succeeded;
        }
    };

    /**
     * @brief Class for managing an OpenGL vertex shader.
     */
    class Vertex_Shader : public Shader
    {
    public:
        /**
         * @brief Constructor for the Vertex_Shader class.
         * @param source_code Vector of vertex shader source code.
         */
        Vertex_Shader(const vector<string>& source_code) : Shader(source_code, GL_VERTEX_SHADER)
        {
        }
    };

    /**
     * @brief Class for managing an OpenGL fragment shader.
     */
    class Fragment_Shader : public Shader
    {
    public:
        /**
         * @brief Constructor for the Fragment_Shader class.
         * @param source_code Vector of fragment shader source code.
         */
        Fragment_Shader(const vector<string>& source_code) : Shader(source_code, GL_FRAGMENT_SHADER)
        {
        }
    };

    /**
     * @brief Class for managing an OpenGL shader program.
     */
    class Shader_Program
    {
    private:
        GLuint program_id; ///< Shader program ID.

    public:
        /**
         * @brief Constructor for the Shader_Program class.
         * @param source_code_vertex Vector of vertex shader source code.
         * @param source_code_fragment Vector of fragment shader source code.
         */
        Shader_Program(const vector<string>& source_code_vertex, const vector<string>& source_code_fragment);

        Shader_Program() = delete; ///< Deleted default constructor.

        /**
         * @brief Destructor for the Shader_Program class.
         */
        ~Shader_Program()
        {
            glDeleteProgram(program_id);
        }

        /**
         * @brief Uses the shader program.
         */
        void use() const
        {
            glUseProgram(program_id);
        }

        /**
         * @brief Gets the shader program ID.
         * @return Shader program ID.
         */
        GLuint get_id() const
        {
            return program_id;
        }

        /**
         * @brief Gets the uniform location in the shader program.
         * @param uniform_name Name of the uniform.
         * @return Uniform location.
         */
        GLuint get_uniform_location(string uniform_name) const
        {
            return glGetUniformLocation(program_id, uniform_name.c_str());
        }

    private:
        Shader_Program(const Shader_Program&) = delete; ///< Deleted copy constructor.
        Shader_Program& operator=(const Shader_Program&) = delete; ///< Deleted copy assignment operator.

        /**
         * @brief Initializes the shader program.
         * @param vertex_shader_id Vertex shader ID.
         * @param fragment_shader_id Fragment shader ID.
         */
        void initialize(GLuint vertex_shader_id, GLuint fragment_shader_id);

        /**
         * @brief Displays linkage errors.
         */
        void show_linkage_error();
    };
}
