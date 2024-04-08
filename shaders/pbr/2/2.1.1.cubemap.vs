#version 330 core
layout (location = 0) in vec3 aPos;

out vec3 WorldPos;

uniform mat4 projection;
uniform mat4 view;

void main()
{
    WorldPos = aPos;
    gl_Position =  projection * view * vec4(WorldPos, 1.0);
}
//�� equirectangular ͼ��ת��Ϊ��������ͼ��������Ҫ��Ⱦһ������λ�������岢�� equirectangular ӳ����ڲ�ͶӰ����������������ϣ���Ϊ�������ÿ���������� 6 ��ͼ����Ϊ��������ͼ�档 ���������Ķ�����ɫ��ֻ�Ǽ򵥵���Ⱦ�����岢���䱾��λ����Ϊ 3D �����������ݸ�fragment shaderƬ����ɫ��
             
//ԭ�����ӣ�https://blog.csdn.net/u013617851/article/details/122460724