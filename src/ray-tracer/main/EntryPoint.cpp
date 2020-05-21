//#include <thirdparty\glad\include\glad\glad.h>
//#include <thirdparty/glfw-3.3/include/GLFW/glfw3.h>

//#define _CRTDBG_MAP_ALLOC  
//#include <stdlib.h>  
//#include <crtdbg.h>  

#include <iostream>
#include <ray-tracer/editor/AssetImporter.h>
#include <ray-tracer/editor/Editor.h>
#include <ray-tracer/main/Window.h>
#include <ray-tracer/editor/Logger.h>
#include <ray-tracer/editor/Shader.h>

#include <ray-tracer/main/Scene.h>

// settings
const unsigned int SCR_WIDTH = 1920;
const unsigned int SCR_HEIGHT = 1440;

int main()
{
	Chroma::Logger::Init("1.12.0");

	Chroma::Window window = Chroma::Window(-1, -1, "Chroma Ray Tracer");
	//Chroma::RayTracer* rt = new Chroma::RayTracer();

	// build and compile our shader program
	// ------------------------------------
	// Create and compile our GLSL program from the shaders
	Chroma::Shader* shader = Chroma::Shader::ReadAndBuildShaderFromFile("../../assets/shaders/phong.vert", "../../assets/shaders/phong.frag");

	std::shared_ptr<Chroma::Scene> scene;
	Chroma::Settings s;
	scene = std::make_shared<Chroma::Scene>(*(Chroma::AssetImporter::LoadSceneFromXML(shader, "../../assets/scenes/hw5/cube_directional.xml", &s)));
	//init editor
	Chroma::Editor editor(&window, scene.get(), s);

	//editor.TogglePreviewRender(false);

	//Model import
	/*auto d_mesh = std::make_shared<Chroma::Mesh>(*Chroma::AssetImporter::LoadMeshFromOBJ("../../assets/models/dragon.obj"));
	//Chroma::Texture* text = new Chroma::Texture("../../assets/textures/white.png");
	std::shared_ptr<Chroma::Dielectric> mat = std::make_shared<Chroma::Dielectric>("u_Material",
		glm::vec3(), glm::vec3(), glm::vec3(), 0.0f);
	mat->m_refraction_ind = 1.56;
	mat->m_absorption_coeff = { 0.15f, 0.15f, 0.0f };
	std::shared_ptr<Chroma::SceneObject> dragon = std::make_shared<Chroma::SceneObject>(d_mesh, "dragon");
	dragon->SetMaterial(mat);


	//Model import
	auto t_mesh = std::make_shared<Chroma::Mesh>(*Chroma::AssetImporter::LoadMeshFromOBJ("../../assets/models/utah_teapot.obj"));
	std::shared_ptr<Chroma::Material> mat2 = std::make_shared<Chroma::Material>("u_Material",
		glm::vec3({ 0.8f, 0.8f, 0.8f }), glm::vec3({ 0.8f, 0.8f, 0.8f }), glm::vec3({ 1.0f, 1.0f, 1.0f }), 90.0f);
	std::shared_ptr<Chroma::SceneObject> teapot = std::make_shared<Chroma::SceneObject>(t_mesh, "teapot");
	//teapot->SmoothNormals();
	teapot->SetMaterial(mat2);

	//Model import
	auto r_mesh = std::make_shared<Chroma::Mesh>(*Chroma::AssetImporter::LoadMeshFromOBJ("../../assets/models/rabbit.obj"));
	std::shared_ptr<Chroma::Conductor> mat3 = std::make_shared<Chroma::Conductor>("u_Material",
		glm::vec3({ 0.0f, 0.0f, 0.0f }), glm::vec3({ 0.0f, 0.0f, 0.0f }), glm::vec3({ 0.71f, 0.651f, 0.26f }) / 8.0f, 1.0f);
	mat3->m_absorption_ind = 3.1350;
	mat3->m_roughness = 0.5f;
	mat3->m_refraction_ind = 0.46192f;
	mat3->m_mirror_reflec = { 0.71f, 0.651f, 0.26f };
	std::shared_ptr<Chroma::SceneObject> rabbit = std::make_shared<Chroma::SceneObject>(r_mesh, "rabbit");
	rabbit->SetMaterial(mat3);

	//Model import
	auto o_mesh = std::make_shared<Chroma::Mesh>(*Chroma::AssetImporter::LoadMeshFromOBJ("../../assets/models/torus_knot.obj"));
	std::shared_ptr<Chroma::Conductor> mat4 = std::make_shared<Chroma::Conductor>("u_Material",
		glm::vec3({ 0.0f, 0.0f, 0.0f }), glm::vec3({ 0.0f, 0.0f, 0.0f }), glm::vec3({ 0.75f, 0.816f, 0.82f }) / 8.0f, 1.0f);
	mat4->m_absorption_ind = 4.0504;
	mat4->m_roughness = 0.05f;
	mat4->m_refraction_ind = 1.6912;
	mat4->m_mirror_reflec = { 0.75f, 0.816f, 0.82f };
	std::shared_ptr<Chroma::SceneObject> torus = std::make_shared<Chroma::SceneObject>(o_mesh, "torus");
	torus->SetMaterial(mat4);

	//Model import
	auto b_mesh = std::make_shared<Chroma::Mesh>(*Chroma::AssetImporter::LoadMeshFromOBJ("../../assets/models/bridge.obj"));
	std::shared_ptr<Chroma::Dielectric> mat5 = std::make_shared<Chroma::Dielectric>("u_Material",
		glm::vec3({ 0.0f, 0.0f, 0.0f }), glm::vec3({ 0.0f, 0.0f, 0.0f }), glm::vec3({ 0, 0, 0 }), 1.0f);
	mat5->m_refraction_ind = 3.9690;
	mat5->m_absorption_coeff = { 0.0, 0.021427, 0.021427 };
	std::shared_ptr<Chroma::SceneObject> bridge = std::make_shared<Chroma::SceneObject>(b_mesh, "bridge");
	bridge->SetMaterial(mat5);
	bridge->SmoothNormals();

	scene->AddSceneObject(dragon->GetName(), dragon);
	scene->AddSceneObject(teapot->GetName(), teapot);
	scene->AddSceneObject(rabbit->GetName(), rabbit);
	scene->AddSceneObject(torus->GetName(), torus);
	scene->AddSceneObject(bridge->GetName(), bridge);*/

	glEnable(GL_DEPTH_TEST);

	/*Chroma::Camera* cam = new Chroma::Camera(1.0f * 1280,
		1.0f * 720, 0.1f, 300.0f, 45.0f);
	//OrthographicCamera cam2(-0.8f, 0.8f, -0.9, 0.9, -10, 10);
	cam->SetPosition({ -0.0f, 0.0f, 50.0f });
	cam->SetUp({ -0.0f, 1.0f, 0.0f });
	cam->SetGaze(cam->GetPosition() + glm::vec3(0.0, 0.0, -1.0f));
	//scene->AddCamera("pers-cam", cam);

	std::shared_ptr<Chroma::PointLight> pl = std::make_shared<Chroma::PointLight>(glm::vec3(0.0f, 0.0f, 40.0f), glm::vec3(0.1f, 0.1f, 0.1f),
		glm::vec3(0.6f, 0.5f, 0.6f), glm::vec3(1.0f, 1.0f, 1.0f));

	std::shared_ptr<Chroma::DirectionalLight> dl = std::make_shared<Chroma::DirectionalLight>(glm::vec3(-30.0f, 0.0f, -40.0f),
		glm::vec3(0.1f, 0.1f, 0.1f), glm::vec3(0.5f, 0.5f, 0.5f), glm::vec3(1.0f, 1.0f, 1.0f));

	std::shared_ptr<Chroma::SpotLight> sl = std::make_shared<Chroma::SpotLight>(glm::vec3(-20.0f, 0.0f, 40.0f), glm::vec3(0.0f, 0.0f, -1.0f),
		glm::vec3(0.5f, 0.5f, 0.5f), glm::vec3(0.5f, 0.5f, 0.5f), glm::vec3(1.0f, 1.0f, 1.0f));*/

		//scene->AddLight("point 1", pl);
		//scene->AddLight("spot 1", sl);
		//scene->AddLight("directional 1", dl);

	/*dragon->SetScale({ 11, 11, 11 });
	dragon->SetPosition({ -1.7,-7.150, 1.8 });
	dragon->SetRotation({0, 125, 0});

	teapot->SetScale({ 2.2,2.2,2.2 });
	teapot->SetPosition({ 3.550,-10.25, 4.5 });
	teapot->SetRotation({ 0,56,0 });

	rabbit->SetScale({ 0.170,0.170, 0.170 });
	rabbit->SetPosition({ -7.250, -10.25, 1.35 });
	rabbit->SetRotation({ -90, 35,0 });

	torus->SetScale({ 1.3, 1.3, 1.3 });
	torus->SetPosition({ 6.800, -7.9, 2.5 });
	torus->SetRotation({ 27, -12,0 });

	bridge->SetScale({ 0.01, 0.05, 0.03 });
	bridge->SetPosition({ 0.3, -9.6,-4.5 });
	bridge->SetRotation({ 0, 90,0 });*/


	/*glm::vec4 dir({ 0.0f, 0.0f, 0.0f, 1.0f });

	teapot->SetScale({ 10.0f, 10.0f, 10.0f });
	teapot->SetPosition({ 0.0f, -9.0f, 0.0f });
	teapot->SetRotation(glm::quat({ glm::radians(0.0f), glm::radians(45.0f), glm::radians(0.0f) }));

	/*box->SetScale({ .9f, .9f, .9f });
	box->SetPosition({ 35.0f, 0.0f, 0.0f });
	box->RotateAngleAxis(glm::radians(0.0), glm::vec3(0.0, 0.0, 1.0));*/

	editor.SetScene(scene.get());

	/*tinyxml2::XMLText* textNode = doc.FirstChildElement("Scene")->FirstChildElement("Cameras")->FirstChild()->FirstChildElement("Position")->FirstChild()->ToText();
	CH_TRACE(textNode->Value());*/

	glClearColor(scene->m_sky_color.x, scene->m_sky_color.y, scene->m_sky_color.z, scene->m_sky_color.w);
	// render loop
	// -----------
	while (!window.ShouldClose())
	{
		editor.OnUpdate();
	}

	// glfw: terminate, clearing all previously allocated GLFW resources.
	// ------------------------------------------------------------------
	glfwTerminate();

	//_CrtDumpMemoryLeaks();
	return 0;
}