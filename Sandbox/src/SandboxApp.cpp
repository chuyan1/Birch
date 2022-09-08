#include <Birch.h>

#include "imgui/imgui.h"

#include <glm/gtc/matrix_transform.hpp>

class ExampleLayer : public Birch::Layer
{
public:
	ExampleLayer(std::string layerName)
		:Layer(layerName), m_Camera(-1.6f, 1.6f, -0.9f, 0.9f), m_CameraPosition(0.0f)
	{
		// 矩形
		m_SquareVA.reset(Birch::VertexArray::Creat());

		float squareVertices[3 * 4] = {
			-0.5f, -0.5f, 0.0f,
			 0.5f, -0.5f, 0.0f,
			 0.5f,  0.5f, 0.0f,
			-0.5f,  0.5f, 0.0f
		};

		std::shared_ptr<Birch::VertexBuffer> squareVB;
		squareVB.reset(Birch::VertexBuffer::Create(squareVertices, sizeof(squareVertices)));
		squareVB->SetLayout({
			{Birch::ShaderDataType::Float3, "a_Position"}
		});
		m_SquareVA->AddVertexBuffer(squareVB);

		uint32_t squareIndices[6] = {0, 1, 2, 2, 3, 0};
		std::shared_ptr<Birch::IndexBuffer> squareIB;
		squareIB.reset(Birch::IndexBuffer::Create(squareIndices, sizeof(squareIndices) / sizeof(uint32_t)));
		m_SquareVA->SetIndexBuffer(squareIB);

		std::string blueShaderVertexSrc = R"(
				#version 330 core

				layout(location = 0) in vec3 a_Position;
				
				uniform mat4 u_ViewProjection;
				uniform mat4 u_Transform;

				out vec3 v_Position;

				void main()
				{
					v_Position = a_Position;
					gl_Position = u_ViewProjection * u_Transform * vec4(a_Position, 1.0);
				}

		)";
		std::string blueShaderFragmentSrc = R"(
				#version 330 core

				layout(location = 0) out vec4 color;
				
				in vec3 v_Position;

				void main()
				{
					color = vec4(0.2, 0.3, 0.8, 1.0);
				}

		)";
		m_BlueShader.reset(new Birch::Shader(blueShaderVertexSrc, blueShaderFragmentSrc));

		// 三角形
		m_VertexArray.reset(Birch::VertexArray::Creat());

		float vertices[3 * 7] = {
			-0.5f, -0.5f, 0.0f, 0.8f, 0.2f, 0.8f, 1.0f,
			 0.5f, -0.5f, 0.0f, 0.2f, 0.3f, 0.8f, 1.0f,
			 0.0f,  0.5f, 0.0f, 0.8f, 0.8f, 0.2f, 1.0f
		};

		std::shared_ptr<Birch::VertexBuffer> vertexBuffer;
		vertexBuffer.reset(Birch::VertexBuffer::Create(vertices, sizeof(vertices)));

		vertexBuffer->SetLayout({
			{ Birch::ShaderDataType::Float3, "a_Position" },
			{ Birch::ShaderDataType::Float4, "a_Color" }
			});
		m_VertexArray->AddVertexBuffer(vertexBuffer);

		uint32_t indices[3] = { 0, 1, 2 };
		std::shared_ptr<Birch::IndexBuffer> indexBuffer;
		indexBuffer.reset(Birch::IndexBuffer::Create(indices, sizeof(indices) / sizeof(uint32_t)));
		m_VertexArray->SetIndexBuffer(indexBuffer);

		std::string vertexSrc = R"(
				#version 330 core

				layout(location = 0) in vec3 a_Position;
				layout(location = 1) in vec4 a_Color;

				uniform mat4 u_ViewProjection;
				uniform mat4 u_Transform;

				out vec3 v_Position;
				out vec4 v_Color;

				void main()
				{
					v_Position = a_Position;
					v_Color = a_Color;
					gl_Position = u_ViewProjection * u_Transform * vec4(a_Position, 1.0);
				}

		)";
		std::string fragmentSrc = R"(
				#version 330 core

				layout(location = 0) out vec4 color;				
				
				in vec3 v_Position;
				in vec4 v_Color;

				void main()
				{
					color = vec4(v_Position * 0.5 + 0.5, 1.0);
					color = v_Color;
				}

		)";
		m_Shader.reset(new Birch::Shader(vertexSrc, fragmentSrc));
	}

	void OnUpdate(Birch::Timestep ts) override
	{
		if (Birch::Input::IsKeyPressed(BC_KEY_LEFT))
			m_CameraPosition.x -= m_CameraMoveSpeed * ts;
		else if (Birch::Input::IsKeyPressed(BC_KEY_RIGHT))
			m_CameraPosition.x += m_CameraMoveSpeed * ts;

		if (Birch::Input::IsKeyPressed(BC_KEY_UP))
			m_CameraPosition.y += m_CameraMoveSpeed * ts;
		else if (Birch::Input::IsKeyPressed(BC_KEY_DOWN))
			m_CameraPosition.y -= m_CameraMoveSpeed * ts;

		if (Birch::Input::IsKeyPressed(BC_KEY_A))
			m_CameraPosition.x += m_CameraRotationSpeed * ts;
		else if (Birch::Input::IsKeyPressed(BC_KEY_D))
			m_CameraPosition.x -= m_CameraRotationSpeed * ts;

		Birch::RenderCommand::SetClearColor({ 0.1f, 0.1f, 0.1f, 1 });
		Birch::RenderCommand::clear();

		m_Camera.SetPosition(m_CameraPosition);	// 控制视口的位置
		m_Camera.SetRotation(m_CameraRotation);	// 控制视口旋转

		Birch::Renderer::BeginScene(m_Camera);

		glm::mat4 scale = glm::scale(glm::mat4(1.0f), glm::vec3(1.0f));
		for (int y = 0; y < 20; y++)
		{
			for (int x = 0; x < 20; x++)
			{
				glm::vec3 pos(x * 0.11f, y * 0.11f, 0.0f);
				glm::mat4 transform = glm::translate(glm::mat4(1.0f), pos) * scale;
				Birch::Renderer::Submit(m_BlueShader, m_SquareVA, transform);
			}
		}
		//Birch::Renderer::Submit(m_BlueShader, m_SquareVA);
		Birch::Renderer::Submit(m_Shader, m_VertexArray);

		Birch::Renderer::EndScene();
	}

	virtual void OnImGuiRender() override
	{
// 		ImGui::Begin("Test");
// 		ImGui::Text("Hello, World");
// 		ImGui::End();
	}

	void OnEvent(Birch::Event& event) override
	{
// 		if (event.GetEventType() == Birch::EventType::KeyPressed)
// 		{
// 			Birch::KeyPressedEvent& e = (Birch::KeyPressedEvent&)event;
// 			if (e.GetKeyCode() == BC_KEY_TAB)
// 				BC_TRACE("Tab key is pressed(event)!");
// 			BC_TRACE("{0}", (char)e.GetKeyCode()); 
// 		}
	}

private:
	std::shared_ptr<Birch::Shader> m_Shader;
	std::shared_ptr<Birch::VertexArray> m_VertexArray;

	std::shared_ptr<Birch::Shader> m_BlueShader;
	std::shared_ptr<Birch::VertexArray> m_SquareVA;

	Birch::OrthographicCamera m_Camera;
	glm::vec3 m_CameraPosition;
	float m_CameraMoveSpeed = 5.0f;

	float m_CameraRotation = 0.0f;
	float m_CameraRotationSpeed = 180.f;
};

class Sandbox : public Birch::Application
{
public:
	Sandbox() 
	{
		PushLayer(new ExampleLayer("ExampleLayer"));
	}
	~Sandbox() {}
};

Birch::Application* Birch::CreateApplication()
{
	return new Sandbox();
}