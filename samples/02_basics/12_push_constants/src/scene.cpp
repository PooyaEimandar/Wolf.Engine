#include "pch.h"
#include "scene.h"

using namespace std;
using namespace wolf;
using namespace wolf::system;
using namespace wolf::graphics;

static uint32_t sFPS = 0;
static float sElapsedTimeInSec = 0;
static float sTotalTimeTimeInSec = 0;
//++++++++++++++++++++++++++++++++++++++++++++++++++++
//The following codes have been added for this project
//++++++++++++++++++++++++++++++++++++++++++++++++++++
static float sPush = true;
static float sPushConstantColorEdit[4] = { 0.4f, 0.7f, 0.0f, 1.0f };
//++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++

scene::scene(_In_z_ const std::wstring& pContentPath, _In_ const wolf::system::w_logger_config& pLogConfig) :
    w_game(pContentPath, pLogConfig)
{
	w_graphics_device_manager_configs _config;
	_config.debug_gpu = false;
	w_game::set_graphics_device_manager_configs(_config);

	w_game::set_fixed_time_step(false);
}

scene::~scene()
{
	//release all resources
	release();
}

void scene::initialize(_In_ std::map<int, w_window_info> pOutputWindowsInfo)
{
	// TODO: Add your pre-initialization logic here
	w_game::initialize(pOutputWindowsInfo);
}

void scene::load()
{
	defer(nullptr, [&](...)
	{
		w_game::load();
	});

	const std::string _trace_info = this->name + "::load";

	auto _gDevice = this->graphics_devices[0];
	auto _output_window = &(_gDevice->output_presentation_window);

	w_point_t _screen_size;
	_screen_size.x = _output_window->width;
	_screen_size.y = _output_window->height;

	//initialize viewport
	this->_viewport.y = 0;
	this->_viewport.width = static_cast<float>(_screen_size.x);
	this->_viewport.height = static_cast<float>(_screen_size.y);
	this->_viewport.minDepth = 0;
	this->_viewport.maxDepth = 1;

	//initialize scissor of viewport
	this->_viewport_scissor.offset.x = 0;
	this->_viewport_scissor.offset.y = 0;
	this->_viewport_scissor.extent.width = _screen_size.x;
	this->_viewport_scissor.extent.height = _screen_size.y;

	//define color and depth as an attachments buffers for render pass
	std::vector<std::vector<w_image_view>> _render_pass_attachments;
	for (size_t i = 0; i < _output_window->swap_chain_image_views.size(); ++i)
	{
		_render_pass_attachments.push_back
		(
			//COLOR									   , DEPTH
			{ _output_window->swap_chain_image_views[i], _output_window->depth_buffer_image_view }
		);
	}
	//create render pass
	auto _hr = this->_draw_render_pass.load(
		_gDevice,
		_viewport,
		_viewport_scissor,
		_render_pass_attachments);
	if (_hr == W_FAILED)
	{
		release();
		V(W_FAILED,
			w_log_type::W_ERROR,
			true,
			"creating render pass. graphics device: {} . trace info: {}", _gDevice->get_info(), _trace_info);
	}

	//create semaphore
	_hr = this->_draw_semaphore.initialize(_gDevice);
	if (_hr == W_FAILED)
	{
		release();
		V(W_FAILED,
			w_log_type::W_ERROR,
			true,
			"creating draw semaphore. graphics device: {} . trace info: {}", _gDevice->get_info(), _trace_info);
	}

	//Fence for syncing
	_hr = this->_draw_fence.initialize(_gDevice);
	if (_hr == W_FAILED)
	{
		release();
		V(W_FAILED,
			w_log_type::W_ERROR,
			true,
			"creating draw fence. graphics device: {} . trace info: {}", _gDevice->get_info(), _trace_info);
	}

	//load imgui
	w_imgui::load(
		_gDevice,
		_output_window,
		this->_viewport,
		this->_viewport_scissor,
		nullptr);

	//create two primary command buffers for clearing screen
	auto _swap_chain_image_size = _output_window->swap_chain_image_views.size();
	_hr = this->_draw_command_buffers.load(_gDevice, _swap_chain_image_size);
	if (_hr == W_FAILED)
	{
		release();
		V(W_FAILED,
			w_log_type::W_ERROR,
			true,
			"creating draw command buffers. graphics device: {} . trace info: {}", _gDevice->get_info(), _trace_info);
	}

#ifdef WIN32
	auto _content_path_dir = wolf::system::io::get_current_directoryW() + L"/../../../../samples/02_basics/12_push_constants/src/content/";
#elif defined(__APPLE__)
	auto _content_path_dir = wolf::system::io::get_current_directoryW() + L"/../../../../../samples/02_basics/12_push_constants/src/content/";
#endif // WIN32

	//loading vertex shaders
	_hr = this->_shader.load(_gDevice,
		_content_path_dir + L"shaders/shader.vert.spv",
		w_shader_stage_flag_bits::VERTEX_SHADER);
	if (_hr == W_FAILED)
	{
		release();
		V(W_FAILED,
			w_log_type::W_ERROR,
			true,
			"loading vertex shader. graphics device: {} . trace info: {}", _gDevice->get_info(), _trace_info);
	}

	//loading fragment shader
	_hr = this->_shader.load(_gDevice,
		_content_path_dir + L"shaders/shader.frag.spv",
		w_shader_stage_flag_bits::FRAGMENT_SHADER);
	if (_hr == W_FAILED)
	{
		release();
		V(W_FAILED,
			w_log_type::W_ERROR,
			true,
			"loading fragment shader. graphics device: {} . trace info: {}", _gDevice->get_info(), _trace_info);
	}

	//load texture
	_hr = this->_texture.initialize(_gDevice);
	if (_hr == W_FAILED)
	{
		release();
		V(W_FAILED,
			w_log_type::W_ERROR,
			true,
			"loading texture. graphics device: {} . trace info: {}", _gDevice->get_info(), _trace_info);
	}
	//load texture from file
	_hr = this->_texture.load_texture_2D_from_file(content_path + L"../Logo.jpg", true);
	if (_hr == W_FAILED)
	{
		release();
		V(W_FAILED,
			w_log_type::W_ERROR,
			true,
			"loading Logo.jpg texture. graphics device: {} . trace info: {}", _gDevice->get_info(), _trace_info);
	}

	//just we need vertex position color
	this->_mesh.set_vertex_binding_attributes(w_vertex_declaration::VERTEX_POSITION_UV);

	w_shader_binding_param _shader_param;
	_shader_param.index = 0;
	_shader_param.type = w_shader_binding_type::SAMPLER2D;
	_shader_param.stage = w_shader_stage_flag_bits::FRAGMENT_SHADER;
	_shader_param.image_info = this->_texture.get_descriptor_info();

	_hr = this->_shader.set_shader_binding_params(
		{
			_shader_param
		});
	if (_hr == W_FAILED)
	{
		release();
		V(W_FAILED,
			w_log_type::W_ERROR,
			true,
			"setting shader binding param. graphics device: {} . trace info: {}", _gDevice->get_info(), _trace_info);
	}

	//loading pipeline cache
	std::string _pipeline_cache_name = "pipeline_cache";
	if (w_pipeline::create_pipeline_cache(_gDevice, _pipeline_cache_name) == W_FAILED)
	{
		logger.error("could not create pipeline cache");
		_pipeline_cache_name.clear();
	}

	w_push_constant_range _push_constants_buffer_range;
	_push_constants_buffer_range.offset = 0;
	_push_constants_buffer_range.size = static_cast<uint32_t>(4 * sizeof(float));
	_push_constants_buffer_range.stageFlags = w_shader_stage_flag_bits::VERTEX_SHADER;

	_hr = this->_pipeline.load(_gDevice,
		this->_mesh.get_vertex_binding_attributes(),
		w_primitive_topology::TRIANGLE_LIST,
		&this->_draw_render_pass,
		&this->_shader,
		{ this->_viewport },
		{ this->_viewport_scissor },
		"pipeline_cache",
		{},
		{ _push_constants_buffer_range });
	if (_hr == W_FAILED)
	{
		release();
		V(W_FAILED,
			w_log_type::W_ERROR,
			true,
			"creating solid pipeline. graphics device: {} . trace info: {}", _gDevice->get_info(), _trace_info);
	}

	std::vector<float> _vertex_data =
	{
		-0.7f, -0.7f,	0.0f,		//pos0
		 0.0f,  0.0f,               //uv0
		-0.7f,  0.7f,	0.0f,		//pos1
		 0.0f,  1.0f,               //uv1
		 0.7f,  0.7f,	0.0f,		//pos2
		 1.0f,  1.0f,           	//uv2
		 0.7f, -0.7f,	0.0f,		//pos3
		 1.0f,  0.0f,               //uv3
	};

	std::vector<uint32_t> _index_data = { 0, 1, 3, 3, 1, 2 };

	this->_mesh.set_texture(&this->_texture);
	_hr = this->_mesh.load(_gDevice,
		_vertex_data.data(),
		static_cast<uint32_t>(_vertex_data.size() * sizeof(float)),
		static_cast<uint32_t>(_vertex_data.size()),
		_index_data.data(),
		static_cast<uint32_t>(_index_data.size()));
	if (_hr == W_FAILED)
	{
		release();
		V(W_FAILED,
			w_log_type::W_ERROR,
			true,
			"loading mesh. graphics device: {} . trace info: {}", _gDevice->get_info(), _trace_info);
	}
}

W_RESULT scene::_build_draw_command_buffers()
{
	const std::string _trace_info = this->name + "::build_draw_command_buffers";
	W_RESULT _hr = W_PASSED;

	auto _gDevice = this->get_graphics_device(0);
	auto _size = this->_draw_command_buffers.get_commands_size();

	for (uint32_t i = 0; i < _size; ++i)
	{
		auto _cmd = this->_draw_command_buffers.get_command_at(i);
		this->_draw_command_buffers.begin(i);
		{
			this->_draw_render_pass.begin(
				i,
				_cmd,
				w_color::CORNFLOWER_BLUE(),
				1.0f,
				0.0f);
			{
				this->_pipeline.bind(_cmd, w_pipeline_bind_point::GRAPHICS);
				//++++++++++++++++++++++++++++++++++++++++++++++++++++
				//The following codes have been added for this project
				//++++++++++++++++++++++++++++++++++++++++++++++++++++
				this->_pipeline.set_push_constant_buffer(
					_cmd,
					w_shader_stage_flag_bits::VERTEX_SHADER,
					0, 
					static_cast<uint32_t>(4 * sizeof(float)), 
					sPushConstantColorEdit);
				//++++++++++++++++++++++++++++++++++++++++++++++++++++
				//++++++++++++++++++++++++++++++++++++++++++++++++++++
				_hr = this->_mesh.draw(_cmd, nullptr, 0, false);
				if (_hr == W_FAILED)
				{
					V(W_FAILED,
						w_log_type::W_ERROR,
						"drawing mesh. graphics device: {} . trace info: {}", _gDevice->get_info(), _trace_info);
				}
			}
			this->_draw_render_pass.end(_cmd);
		}
		this->_draw_command_buffers.end(i);
	}
	return _hr;
}

void scene::update(_In_ const wolf::system::w_game_time& pGameTime)
{
	if (w_game::exiting) return;
	const std::string _trace_info = this->name + "::update";

    sFPS = pGameTime.get_frames_per_second();
    sElapsedTimeInSec = pGameTime.get_elapsed_seconds();
    sTotalTimeTimeInSec = pGameTime.get_total_seconds();

    w_imgui::new_frame(sElapsedTimeInSec, [this]()
    {
        _update_gui();
    });
    
	w_game::update(pGameTime);
}

W_RESULT scene::render(_In_ const wolf::system::w_game_time& pGameTime)
{
	if (w_game::exiting) return W_PASSED;

	const std::string _trace_info = this->name + "::render";

	auto _gDevice = this->graphics_devices[0];
	auto _output_window = &(_gDevice->output_presentation_window);
	auto _frame_index = _output_window->swap_chain_image_index;

    if (sPush)
    {
        _build_draw_command_buffers();
        sPush = false;
    }

	w_imgui::render();

	auto _draw_cmd = this->_draw_command_buffers.get_command_at(_frame_index);
	auto _gui_cmd = w_imgui::get_command_buffer_at(_frame_index);

	const std::vector<w_pipeline_stage_flag_bits> _wait_dst_stage_mask =
	{
		w_pipeline_stage_flag_bits::COLOR_ATTACHMENT_OUTPUT_BIT,
	};

	//reset draw fence
	this->_draw_fence.reset();
	if (_gDevice->submit(
		{ &_draw_cmd, &_gui_cmd },//command buffers
		_gDevice->vk_graphics_queue, //graphics queue
		_wait_dst_stage_mask, //destination masks
		{ _output_window->swap_chain_image_is_available_semaphore }, //wait semaphores
		{ _output_window->rendering_done_semaphore }, //signal semaphores
		&this->_draw_fence,
		false) == W_FAILED)
	{
		release();
		V(W_FAILED,
			w_log_type::W_ERROR,
			true,
			"submiting queue for drawing. graphics device: {} . trace info: {}", _gDevice->get_info(), _trace_info);
	}
	this->_draw_fence.wait();
	
	return w_game::render(pGameTime);
}

void scene::on_window_resized(_In_ const uint32_t& pIndex, _In_ const w_point& pNewSizeOfWindow)
{
	w_game::on_window_resized(pIndex, pNewSizeOfWindow);
}

void scene::on_device_lost()
{
	w_game::on_device_lost();
}

ULONG scene::release()
{
    if (this->get_is_released()) return 1;

    //release draw's objects
	this->_draw_fence.release();
	this->_draw_semaphore.release();

	this->_draw_command_buffers.release();
	this->_draw_render_pass.release();

    //release gui's objects
    w_imgui::release();

	this->_shader.release();

    this->_pipeline.release();
    
	this->_mesh.release();
    this->_texture.release();

	return w_game::release();
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++
//The following codes have been added for this project
//++++++++++++++++++++++++++++++++++++++++++++++++++++
bool scene::_update_gui()
{
    //Setting Style
    ImGuiStyle& _style = ImGui::GetStyle();
    _style.Colors[ImGuiCol_Text].x = 1.0f;
    _style.Colors[ImGuiCol_Text].y = 1.0f;
    _style.Colors[ImGuiCol_Text].z = 1.0f;
    _style.Colors[ImGuiCol_Text].w = 1.0f;

    _style.Colors[ImGuiCol_WindowBg].x = 0.0f;
    _style.Colors[ImGuiCol_WindowBg].y = 0.4f;
    _style.Colors[ImGuiCol_WindowBg].z = 1.0f;
    _style.Colors[ImGuiCol_WindowBg].w = 1.0f;

    ImGuiWindowFlags  _window_flags = 0;;
    ImGui::SetNextWindowSize(ImVec2(400, 300), ImGuiSetCond_FirstUseEver);
    bool _is_open = true;
    if (!ImGui::Begin("Wolf.Engine", &_is_open, _window_flags))
    {
        ImGui::End();
        return false;
    }

    ImGui::Text("Press Esc to exit\r\nFPS:%d\r\nFrameTime:%f\r\nTotalTime:%f\r\nMouse Position:%d,%d\r\n",
        sFPS,
        sElapsedTimeInSec,
        sTotalTimeTimeInSec,
        wolf::inputs_manager.mouse.pos_x, wolf::inputs_manager.mouse.pos_y);

    if (ImGui::ColorEdit4("Pick Color", sPushConstantColorEdit))
    {
        sPush = true;
    }

    ImGui::End();

    return true;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++
