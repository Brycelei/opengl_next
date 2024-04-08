#version 330 core
layout (location = 0) in vec3 aPos;

uniform mat4 projection;
uniform mat4 view;

out vec3 WorldPos;

void main()
{
    WorldPos = aPos;
	//// remove translation from the view matrix 移除坐标位移的影响
	mat4 rotView = mat4(mat3(view));
	vec4 clipPos = projection * rotView * vec4(WorldPos, 1.0);

	//这里的 xyww 技巧，它确保渲染的立方体片段的深度值始终以 1.0 结束，即最大深度值，
	//这里的 xyww 技巧，它确保渲染的立方体片段的深度值始终以 1.0 结束，即最大深度值，如立方体贴图章节中所述。 请注意，我们需要将深度比较函数更改为 GL_LEQUAL：如立方体贴图章节中所述。 请注意，我们需要将深度比较函数更改为 GL_LEQUAL：
	gl_Position = clipPos.xyww;
}