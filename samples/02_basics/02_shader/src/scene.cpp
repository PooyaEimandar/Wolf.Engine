#include "pch.h"
#include "scene.h"

using namespace std;
using namespace wolf;
using namespace wolf::system;
using namespace wolf::render::vulkan;

scene::scene(_In_z_ const std::wstring& pContentPath, _In_ const wolf::system::w_logger_config& pLogConfig) :
	w_game(pContentPath, pLogConfig)
{
	w_graphics_device_manager_configs _config;
	_config.debug_gpu = true;
	w_game::set_graphics_device_manager_configs(_config);

	w_game::set_fixed_time_step(false);
}

scene::~scene()
{
	//release all resources
	release();
}

void scene::initialize(_In_ std::map<int, w_present_info> pOutputWindowsInfo)
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
	w_point _offset;
	_offset.x = this->_viewport.x;
	_offset.y = this->_viewport.y;

	w_point_t _size;
	_size.x = this->_viewport.width;
	_size.y = this->_viewport.height;

	//create render pass
	auto _hr = this->_draw_render_pass.load(
		_gDevice,
		_offset,
		_size,
		_render_pass_attachments);
	if (_hr == W_FAILED)
	{
		release();
		V(W_FAILED,
			w_log_type::W_ERROR,
			true,
			"creating render pass. graphics device: {} . trace info: {}", _gDevice->get_info(), _trace_info);
	}

	//Fence for render sync
	_hr = this->_draw_fence.initialize(_gDevice);
	if (_hr == W_FAILED)
	{
		release();
		V(W_FAILED, 
			w_log_type::W_ERROR,
			true,
			"creating draw fence. graphics device: {} . trace info: {}", _gDevice->get_info(), _trace_info);
	}

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

	//++++++++++++++++++++++++++++++++++++++++++++++++++++
	//The following codes have been added for this project
	//++++++++++++++++++++++++++++++++++++++++++++++++++++
#ifdef WIN32
	auto _content_path_dir = wolf::system::io::get_current_directoryW() + L"/../../../../samples/02_basics/02_shader/src/content/";
#elif defined(__APPLE__)
	auto _content_path_dir = wolf::system::io::get_current_directoryW() + L"/../../../../../samples/02_basics/02_shader/src/content/";
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

	//loading pipeline cache
	std::string _pipeline_cache_name = "pipeline_cache";
	if (w_pipeline::create_pipeline_cache(_gDevice, _pipeline_cache_name) == W_FAILED)
	{
		logger.error("could not create pipeline cache");
		_pipeline_cache_name.clear();
	}

	//just we need vertex position
	w_vertex_binding_attributes _vba(w_vertex_declaration::NOT_DEFINED);

	//dynamic states
	std::vector<w_dynamic_state> _dynamic_states =
	{
		VIEWPORT,
		SCISSOR,
	};

	_hr = this->_pipeline.load(_gDevice,
		_vba,
		w_primitive_topology::TRIANGLE_LIST,
		&this->_draw_render_pass,
		&this->_shader,
		{ this->_viewport },
		{ this->_viewport_scissor },
		_pipeline_cache_name,
		_dynamic_states);

	if (_hr == W_FAILED)
	{
		release();
		V(W_FAILED, 
			w_log_type::W_ERROR,
			true, 
			"creating pipeline. graphics device: {} . trace info: {}", _gDevice->get_info(), _trace_info);
	}

	//++++++++++++++++++++++++++++++++++++++++++++++++++++
	//++++++++++++++++++++++++++++++++++++++++++++++++++++

	_build_draw_command_buffers();
}

W_RESULT scene::_build_draw_command_buffers()
{
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
                //++++++++++++++++++++++++++++++++++++++++++++++++++++
                //The following codes have been added for this project
                //++++++++++++++++++++++++++++++++++++++++++++++++++++
				this->_viewport.set(_cmd);
				this->_viewport_scissor.set(_cmd);
                this->_pipeline.bind(_cmd, w_pipeline_bind_point::GRAPHICS);
				_gDevice->draw(_cmd, 3, 1, 0, 0 );
                //++++++++++++++++++++++++++++++++++++++++++++++++++++
                //++++++++++++++++++++++++++++++++++++++++++++++++++++
			}
			this->_draw_render_pass.end(_cmd);
		}
		this->_draw_command_buffers.end(i);
	}
	return W_PASSED;
}

void scene::update(_In_ const wolf::system::w_game_time& pGameTime)
{
	if (w_game::exiting) return;
    const std::string _trace_info = this->name + "::update";
    
	w_game::update(pGameTime);
}

W_RESULT scene::render(_In_ const wolf::system::w_game_time& pGameTime)
{
	if (w_game::exiting) return W_PASSED;

	const std::string _trace_info = this->name + "::render";

	auto _gDevice = this->graphics_devices[0];
	auto _output_window = &(_gDevice->output_presentation_window);
	auto _frame_index = _output_window->swap_chain_image_index;

	std::vector<w_pipeline_stage_flag_bits> _wait_dst_stage_mask =
	{
		w_pipeline_stage_flag_bits::COLOR_ATTACHMENT_OUTPUT_BIT,
	};

	//set active command buffer
	auto _cmd = this->_draw_command_buffers.get_command_at(_frame_index);
	//reset draw fence
	this->_draw_fence.reset();
	if (_gDevice->submit(
		{ &_cmd },//command buffers
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
			"submiting queue for drawing gui. graphics device: {} . trace info: {}", _gDevice->get_info(), _trace_info);
	}
	// Wait for fence to signal that all command buffers are ready
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

	this->_draw_fence.release();

	this->_draw_command_buffers.release();
	this->_draw_render_pass.release();

	//++++++++++++++++++++++++++++++++++++++++++++++++++++
	//The following codes have been added for this project
	//++++++++++++++++++++++++++++++++++++++++++++++++++++
	this->_shader.release();
	this->_pipeline.release();
	//++++++++++++++++++++++++++++++++++++++++++++++++++++
	//++++++++++++++++++++++++++++++++++++++++++++++++++++

	return w_game::release();
}
