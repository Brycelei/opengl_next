#version 330 core
layout (location = 0) in vec3 aPos;

uniform mat4 projection;
uniform mat4 view;

out vec3 WorldPos;

void main()
{
    WorldPos = aPos;
	//// remove translation from the view matrix �Ƴ�����λ�Ƶ�Ӱ��
	mat4 rotView = mat4(mat3(view));
	vec4 clipPos = projection * rotView * vec4(WorldPos, 1.0);

	//����� xyww ���ɣ���ȷ����Ⱦ��������Ƭ�ε����ֵʼ���� 1.0 ��������������ֵ��
	//����� xyww ���ɣ���ȷ����Ⱦ��������Ƭ�ε����ֵʼ���� 1.0 ��������������ֵ������������ͼ�½��������� ��ע�⣬������Ҫ����ȱȽϺ�������Ϊ GL_LEQUAL������������ͼ�½��������� ��ע�⣬������Ҫ����ȱȽϺ�������Ϊ GL_LEQUAL��
	gl_Position = clipPos.xyww;
}