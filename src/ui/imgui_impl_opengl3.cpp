// modified by mwthinker
// dear imgui: Renderer for modern OpenGL with shaders / programmatic pipeline
// - Desktop GL: 3.x 4.x
// - Embedded GL: ES 2.0 (WebGL 1.0), ES 3.0 (WebGL 2.0)
// This needs to be used along with a Platform Binding (e.g. GLFW, SDL, Win32, custom..)

// Implemented features:
//  [X] Renderer: User texture binding. Use 'GLuint' OpenGL texture identifier as void*/ImTextureID. Read the FAQ about ImTextureID in imgui.cpp.

// You can copy and use unmodified imgui_impl_* files in your project. See main.cpp for an example of using this.
// If you are new to dear imgui, read examples/README.txt and read the documentation at the top of imgui.cpp.
// https://github.com/ocornut/imgui

//----------------------------------------
// OpenGL    GLSL      GLSL
// version   version   string
//----------------------------------------
//  3.3       330       "#version 330 core"

#include "imgui.h"
#include "imgui_impl_opengl3.h"
#include "../logger.h"
#include "../types.h"
#include "imguishader.h"

#include <sdl/vertexarrayobject.h>
#include <sdl/window.h>

#if defined(__APPLE__)
#include "TargetConditionals.h"
#endif

#include <sdl/opengl.h>

// OpenGL Data
namespace {
	
	GLuint       g_FontTexture = 0;
	unsigned int g_VboHandle = 0, g_ElementsHandle = 0;	

	tetris::ImGuiShader shader;
	
	bool CheckShader(GLuint handle, const char* desc) {
		GLint status = 0, log_length = 0;
		glGetShaderiv(handle, GL_COMPILE_STATUS, &status);
		glGetShaderiv(handle, GL_INFO_LOG_LENGTH, &log_length);
		if ((GLboolean)status == GL_FALSE) {
			tetris::logger()->error("[ImGui] ImGui_ImplOpenGL3_CreateDeviceObjects: failed to compile {}!", desc);
		}
		if (log_length > 0) {
			ImVector<char> buf;
			buf.resize((int)(log_length + 1));
			glGetShaderInfoLog(handle, log_length, NULL, (GLchar*)buf.begin());
			fprintf(stderr, "%s\n", buf.begin());
			tetris::logger()->error("[ImGui] {}", buf.begin());
		}
		return (GLboolean)status == GL_TRUE;
	}
	
	bool CheckProgram(GLuint handle, const char* desc) {
		GLint status = 0, log_length = 0;
		glGetProgramiv(handle, GL_LINK_STATUS, &status);
		glGetProgramiv(handle, GL_INFO_LOG_LENGTH, &log_length);
		if ((GLboolean)status == GL_FALSE) {
			//tetris::logger()->error("[ImGui] ImGui_ImplOpenGL3_CreateDeviceObjects: failed to link {}! (with GLSL '{}')", desc, g_GlslVersionString);
		}
		if (log_length > 0) {
			ImVector<char> buf;
			buf.resize((int)(log_length + 1));
			glGetProgramInfoLog(handle, log_length, NULL, (GLchar*)buf.begin());
			tetris::logger()->error("[ImGui] {}", buf.begin());
		}
		return (GLboolean)status == GL_TRUE;
	}

	constexpr const GLchar* getVertexShaderGlsl_330() {
		return
			"#version 330\n"
			"uniform mat4 ProjMtx;\n"
			"in vec2 Position;\n"
			"in vec2 UV;\n"
			"in vec4 Color;\n"
			"out vec2 Frag_UV;\n"
			"out vec4 Frag_Color;\n"
			"void main()\n"
			"{\n"
			"    Frag_UV = UV;\n"
			"    Frag_Color = Color;\n"
			"    gl_Position = ProjMtx * vec4(Position.xy,0,1);\n"
			"}\n";
	}

	constexpr const GLchar* getFragmentShaderGlsl_330() {
		return
			"#version 330\n"
			"uniform sampler2D Texture;\n"
			"in vec2 Frag_UV;\n"
			"in vec4 Frag_Color;\n"
			"out vec4 Out_Color;\n"
			"void main()\n"
			"{\n"
			"    Out_Color = Frag_Color * texture(Texture, Frag_UV.st);\n"
			"}\n";
	}

	constexpr int getGlslVersion3(int minorVersion) {
		switch (minorVersion) {
			case 0:
				return 130;
			case 1:
				return 140;
			case 2:
				return 150;
			case 3:
				return 330;
		}
		return -1;
	}

	constexpr int getGlslVersion(int majorVersion, int minorVersion) {
		switch (majorVersion) {
			case 3:
				return getGlslVersion3(minorVersion);
		}
		return -1;
	}

	constexpr std::pair<const GLchar*, const GLchar*> getShader(int glslVersion) {
		if (glslVersion == 330) {
			return { getVertexShaderGlsl_330(), getFragmentShaderGlsl_330() };
		}
		return { "", "" };
	}

	struct GlState {
		GLenum last_active_texture;

		GLint last_program;
		GLint last_texture;
#ifdef GL_SAMPLER_BINDING
		GLint last_sampler;
#endif
		GLint last_array_buffer;
		GLint last_vertex_array;
#ifdef GL_POLYGON_MODE
		GLint last_polygon_mode[2];
#endif
		GLint last_viewport[4];
		GLint last_scissor_box[4];
		GLenum last_blend_src_rgb;
		GLenum last_blend_dst_rgb;
		GLenum last_blend_src_alpha;
		GLenum last_blend_dst_alpha;
		GLenum last_blend_equation_rgb;
		GLenum last_blend_equation_alpha;
		GLboolean last_enable_blend;
		GLboolean last_enable_cull_face;
		GLboolean last_enable_depth_test;
		GLboolean last_enable_scissor_test;
		bool clip_origin_lower_left;
#if defined(GL_CLIP_ORIGIN) && !defined(__APPLE__)
		GLenum last_clip_origin;
#endif
	};

	GlState glState_;

}

// Functions
bool ImGui_ImplOpenGL3_Init(const char* glsl_version) {
    ImGuiIO& io = ImGui::GetIO();
    io.BackendRendererName = "imgui_impl_opengl3";

    // Make a dummy GL call (we don't actually need the result)
    // IF YOU GET A CRASH HERE: it probably means that you haven't initialized the OpenGL function loader used by this code.
    // Desktop OpenGL 3/4 need a function loader. See the IMGUI_IMPL_OPENGL_LOADER_xxx explanation above.
    GLint current_texture;
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &current_texture);
    return true;
}

void ImGui_ImplOpenGL3_Shutdown() {
    ImGui_ImplOpenGL3_DestroyDeviceObjects();
}

void ImGui_ImplOpenGL3_NewFrame() {
	if (!g_FontTexture) {
		ImGui_ImplOpenGL3_CreateDeviceObjects();
	}
}

void backupGlState() {
	// Backup GL state
	glGetIntegerv(GL_ACTIVE_TEXTURE, (GLint*)& glState_.last_active_texture);
	glActiveTexture(GL_TEXTURE0);
	glGetIntegerv(GL_CURRENT_PROGRAM, &glState_.last_program);
	glGetIntegerv(GL_TEXTURE_BINDING_2D, &glState_.last_texture);
#ifdef GL_SAMPLER_BINDING
	glGetIntegerv(GL_SAMPLER_BINDING, &glState_.last_sampler);
#endif
	glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &glState_.last_array_buffer);
	glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &glState_.last_vertex_array);
#ifdef GL_POLYGON_MODE
	glGetIntegerv(GL_POLYGON_MODE, glState_.last_polygon_mode);
#endif
	glGetIntegerv(GL_VIEWPORT, glState_.last_viewport);
	glGetIntegerv(GL_SCISSOR_BOX, glState_.last_scissor_box);
	glGetIntegerv(GL_BLEND_SRC_RGB, (GLint*)& glState_.last_blend_src_rgb);
	glGetIntegerv(GL_BLEND_DST_RGB, (GLint*)& glState_.last_blend_dst_rgb);
	glGetIntegerv(GL_BLEND_SRC_ALPHA, (GLint*)& glState_.last_blend_src_alpha);
	glGetIntegerv(GL_BLEND_DST_ALPHA, (GLint*)& glState_.last_blend_dst_alpha);
	glGetIntegerv(GL_BLEND_EQUATION_RGB, (GLint*)& glState_.last_blend_equation_rgb);
	glGetIntegerv(GL_BLEND_EQUATION_ALPHA, (GLint*)& glState_.last_blend_equation_alpha);
	glState_.last_enable_blend = glIsEnabled(GL_BLEND);
	glState_.last_enable_cull_face = glIsEnabled(GL_CULL_FACE);
	glState_.last_enable_depth_test = glIsEnabled(GL_DEPTH_TEST);
	glState_.last_enable_scissor_test = glIsEnabled(GL_SCISSOR_TEST);
	glState_.clip_origin_lower_left = true;
#if defined(GL_CLIP_ORIGIN) && !defined(__APPLE__)
	glState_.last_clip_origin = 0; glGetIntegerv(GL_CLIP_ORIGIN, (GLint*)& glState_.last_clip_origin); // Support for GL 4.5's glClipControl(GL_UPPER_LEFT)
	if (glState_.last_clip_origin == GL_UPPER_LEFT) {
		glState_.clip_origin_lower_left = false;
	}
#endif
}

void restoreGlState() {
	// Restore modified GL state
	glUseProgram(glState_.last_program);
	glBindTexture(GL_TEXTURE_2D, glState_.last_texture);
#ifdef GL_SAMPLER_BINDING
	glBindSampler(0, glState_.last_sampler);
#endif
	glActiveTexture(glState_.last_active_texture);

	glBindVertexArray(glState_.last_vertex_array);

	glBindBuffer(GL_ARRAY_BUFFER, glState_.last_array_buffer);
	glBlendEquationSeparate(glState_.last_blend_equation_rgb, glState_.last_blend_equation_alpha);
	glBlendFuncSeparate(glState_.last_blend_src_rgb, glState_.last_blend_dst_rgb, glState_.last_blend_src_alpha, glState_.last_blend_dst_alpha);
	if (glState_.last_enable_blend) {
		glEnable(GL_BLEND);
	} else {
		glDisable(GL_BLEND);
	}

	if (glState_.last_enable_cull_face) {
		glEnable(GL_CULL_FACE);
	} else {
		glDisable(GL_CULL_FACE);
	}
	if (glState_.last_enable_depth_test) {
		glEnable(GL_DEPTH_TEST);
	} else {
		glDisable(GL_DEPTH_TEST);
	}
	if (glState_.last_enable_scissor_test) {
		glEnable(GL_SCISSOR_TEST);
	} else {
		glDisable(GL_SCISSOR_TEST);
	}

#ifdef GL_POLYGON_MODE
	glPolygonMode(GL_FRONT_AND_BACK, (GLenum)glState_.last_polygon_mode[0]);
#endif

	glViewport(glState_.last_viewport[0], glState_.last_viewport[1], (GLsizei) glState_.last_viewport[2], (GLsizei) glState_.last_viewport[3]);
	glScissor(glState_.last_scissor_box[0], glState_.last_scissor_box[1], (GLsizei) glState_.last_scissor_box[2], (GLsizei) glState_.last_scissor_box[3]);
}

// OpenGL3 Render function.
// (this used to be set in io.RenderDrawListsFn and called by ImGui::Render(), but you can now call this directly from your main loop)
// Note that this implementation is little overcomplicated because we are saving/setting up/restoring every OpenGL state explicitly, in order to be able to run within any OpenGL engine that doesn't do so.
void ImGui_ImplOpenGL3_RenderDrawData(ImDrawData* draw_data) {
    // Avoid rendering when minimized, scale coordinates for retina displays (screen coordinates != framebuffer coordinates)
    int fb_width = (int)(draw_data->DisplaySize.x * draw_data->FramebufferScale.x);
    int fb_height = (int)(draw_data->DisplaySize.y * draw_data->FramebufferScale.y);
    if (fb_width <= 0 || fb_height <= 0) {
        return;
	}

	backupGlState();

    // Setup render state: alpha-blending enabled, no face culling, no depth testing, scissor enabled, polygon fill
    glEnable(GL_BLEND);
    glBlendEquation(GL_FUNC_ADD);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_SCISSOR_TEST);
#ifdef GL_POLYGON_MODE
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
#endif

    // Setup viewport, orthographic projection matrix
    // Our visible imgui space lies from draw_data->DisplayPos (top left) to draw_data->DisplayPos+data_data->DisplaySize (bottom right). DisplayMin is typically (0,0) for single viewport apps.
    glViewport(0, 0, (GLsizei) fb_width, (GLsizei) fb_height);

	const auto projMatrix = glm::ortho(draw_data->DisplayPos.x, draw_data->DisplayPos.x + draw_data->DisplaySize.x,
		draw_data->DisplayPos.y + draw_data->DisplaySize.y, draw_data->DisplayPos.y);

	shader.useProgram();
	shader.setMatrix(projMatrix);
	shader.setTexture(false);

#ifdef GL_SAMPLER_BINDING
    glBindSampler(0, 0); // We use combined texture/sampler state. Applications using GL 3.3 may set that otherwise.
#endif
    // Recreate the VAO every time
    // (This is to easily allow multiple GL contexts. VAO are not shared among GL contexts, and we don't track creation/deletion of windows so we don't have an obvious key to use to cache them.)
    //VertexA
	
	GLuint vao_handle = 0;
    glGenVertexArrays(1, &vao_handle);
    glBindVertexArray(vao_handle);

    glBindBuffer(GL_ARRAY_BUFFER, g_VboHandle);
	shader.setVertexAttribPointer();

    // Will project scissor/clipping rectangles into framebuffer space
    ImVec2 clip_off = draw_data->DisplayPos;         // (0,0) unless using multi-viewports
    ImVec2 clip_scale = draw_data->FramebufferScale; // (1,1) unless using retina display which are often (2,2)

    // Render command lists
    for (int n = 0; n < draw_data->CmdListsCount; n++) {
        const ImDrawList* cmd_list = draw_data->CmdLists[n];
        size_t idx_buffer_offset = 0;

        glBindBuffer(GL_ARRAY_BUFFER, g_VboHandle);
        glBufferData(GL_ARRAY_BUFFER, (GLsizeiptr)cmd_list->VtxBuffer.Size * sizeof(ImDrawVert), (const GLvoid*)cmd_list->VtxBuffer.Data, GL_STREAM_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_ElementsHandle);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, (GLsizeiptr)cmd_list->IdxBuffer.Size * sizeof(ImDrawIdx), (const GLvoid*)cmd_list->IdxBuffer.Data, GL_STREAM_DRAW);

        for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; cmd_i++) {
            const ImDrawCmd* pcmd = &cmd_list->CmdBuffer[cmd_i];
            if (pcmd->UserCallback) {
                // User callback (registered via ImDrawList::AddCallback)
                pcmd->UserCallback(cmd_list, pcmd);
            } else {
                // Project scissor/clipping rectangles into framebuffer space
                ImVec4 clip_rect;
                clip_rect.x = (pcmd->ClipRect.x - clip_off.x) * clip_scale.x;
                clip_rect.y = (pcmd->ClipRect.y - clip_off.y) * clip_scale.y;
                clip_rect.z = (pcmd->ClipRect.z - clip_off.x) * clip_scale.x;
                clip_rect.w = (pcmd->ClipRect.w - clip_off.y) * clip_scale.y;

                if (clip_rect.x < fb_width && clip_rect.y < fb_height && clip_rect.z >= 0.0f && clip_rect.w >= 0.0f) {
                    // Apply scissor/clipping rectangle
					if (glState_.clip_origin_lower_left) {
						glScissor((int)clip_rect.x, (int)(fb_height - clip_rect.w), (int)(clip_rect.z - clip_rect.x), (int)(clip_rect.w - clip_rect.y));
					} else {
						glScissor((int)clip_rect.x, (int)clip_rect.y, (int)clip_rect.z, (int)clip_rect.w); // Support for GL 4.5's glClipControl(GL_UPPER_LEFT)
					}
                    // Bind texture, Draw
                    glBindTexture(GL_TEXTURE_2D, (GLuint)(intptr_t) pcmd->TextureId);
                    glDrawElements(GL_TRIANGLES, (GLsizei) pcmd->ElemCount, sizeof(ImDrawIdx) == 2 ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT, (void*)idx_buffer_offset);
                }
            }
            idx_buffer_offset += pcmd->ElemCount * sizeof(ImDrawIdx);
        }
    }
    glDeleteVertexArrays(1, &vao_handle);
    
	restoreGlState();
}

bool ImGui_ImplOpenGL3_CreateFontsTexture() {
    // Build texture atlas
    ImGuiIO& io = ImGui::GetIO();
    unsigned char* pixels;
    int width, height;
    io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);   // Load as RGBA 32-bits (75% of the memory is wasted, but default font is so small) because it is more likely to be compatible with user's existing shaders. If your ImTextureId represent a higher-level concept than just a GL texture id, consider calling GetTexDataAsAlpha8() instead to save on GPU memory.

    // Upload texture to graphics system
    GLint last_texture;
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &last_texture);
    glGenTextures(1, &g_FontTexture);
    glBindTexture(GL_TEXTURE_2D, g_FontTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
#ifdef GL_UNPACK_ROW_LENGTH
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
#endif
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

    // Store our identifier
    io.Fonts->TexID = (ImTextureID)(intptr_t) g_FontTexture;

    // Restore state
    glBindTexture(GL_TEXTURE_2D, last_texture);
    return true;
}

void ImGui_ImplOpenGL3_DestroyFontsTexture() {
    if (g_FontTexture) {
        ImGuiIO& io = ImGui::GetIO();
        glDeleteTextures(1, &g_FontTexture);
        io.Fonts->TexID = 0;
        g_FontTexture = 0;
    }
}

bool ImGui_ImplOpenGL3_CreateDeviceObjects() {
    // Backup GL state
    GLint last_texture, last_array_buffer;
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &last_texture);
    glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &last_array_buffer);

    // Parse GLSL version string
    int glsl_version = getGlslVersion(sdl::Window::getOpenGlMajorVersion(), sdl::Window::getOpenGlMinorVersion());
	auto [vertexShader, fragmentShader] = getShader(glsl_version);

	shader = tetris::ImGuiShader(vertexShader, fragmentShader);

    // Create buffers
    glGenBuffers(1, &g_VboHandle);
    glGenBuffers(1, &g_ElementsHandle);

    ImGui_ImplOpenGL3_CreateFontsTexture();

    // Restore modified GL state
    glBindTexture(GL_TEXTURE_2D, last_texture);
    glBindBuffer(GL_ARRAY_BUFFER, last_array_buffer);
    return true;
}

void ImGui_ImplOpenGL3_DestroyDeviceObjects() {
	if (g_VboHandle) {
		glDeleteBuffers(1, &g_VboHandle);
	}
	if (g_ElementsHandle) {
		glDeleteBuffers(1, &g_ElementsHandle);
	}
	
	g_VboHandle = 0;
	g_ElementsHandle = 0;

	shader = tetris::ImGuiShader();

    ImGui_ImplOpenGL3_DestroyFontsTexture();
}
