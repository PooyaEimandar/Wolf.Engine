import sys, os

#get path of script
_script_path = os.path.realpath(__file__)
_script_dir = os.path.dirname(_script_path)
pyWolfPath = _script_dir

if sys.platform == "linux" or sys.platform == "linux2":
    print "Linux not tested yet"
elif sys.platform == "darwin":
    print "OS X not tested yet"
elif sys.platform == "win32":
    pyWolfPath =  pyWolfPath + "\\..\\..\\..\\..\\bin\\x64\\Debug\\Win32\\"

if pyWolfPath != "" and (not pyWolfPath in sys.path):
    sys.path.append(pyWolfPath)

import ctypes, threading, pyWolf
from math import cos, sin
from PySide import QtGui, QtCore
from PySide.QtGui import *
from PySide.QtCore import *

screen_width = 800
screen_height = 600

class gui(QWidget):
    def __init__(self, parent=None):
        super(gui, self).__init__(parent)

        self.debug_text = ""
        self._label = QLabel()
        self._label.setAlignment(Qt.AlignLeft)
        
        vbox = QVBoxLayout()
        vbox.addWidget(self._label)

        self.setLayout(vbox)

        timer = QTimer(self)
        timer.timeout.connect(self.updateTime)
        timer.start(50)

    def updateTime(self):
        self._label.setText(self.debug_text)
     
class scene(QWidget):
    def __init__(self, pContentPath, pLogPath, pAppName, parent = None):
        super(scene, self).__init__(parent)
        self.__exiting = False
        self._game = pyWolf.framework.w_game(pContentPath, pLogPath, pAppName)
        self._game.set_pre_init_callback(self.pre_init)
        self._game.set_post_init_callback(self.post_init)
        self._game.set_load_callback(self.load)
        self._game.set_update_callback(self.update)
        self._game.set_pre_render_callback(self.pre_render)
        self._game.set_post_render_callback(self.post_render)
        self._gDevice = None
        self._viewport = pyWolf.graphics.w_viewport()
        self._viewport_scissor = pyWolf.graphics.w_viewport_scissor()
        self._draw_command_buffers = pyWolf.graphics.w_command_buffers()
        self._draw_render_pass = pyWolf.graphics.w_render_pass()
        self._draw_fence = pyWolf.graphics.w_fences()
        self._draw_semaphore = pyWolf.graphics.w_semaphore()
        #++++++++++++++++++++++++++++++++++++++++++++++++++++
        #The following codes have been added for this project
        #++++++++++++++++++++++++++++++++++++++++++++++++++++
        self._shape_line = pyWolf.graphics.w_shapes(
            pyWolf.glm.vec3(0.0, 0.0, 0.0), 
            pyWolf.glm.vec3(3.0, 3.0, 3.0), 
            pyWolf.system.w_color.RED())

        self._shape_triangle = pyWolf.graphics.w_shapes(
            pyWolf.glm.vec3(-1.0, 0.0, 0.0), 
            pyWolf.glm.vec3(1.0, 0.0, 0.0), 
            pyWolf.glm.vec3(0.0, 2.0, 0.0), 
            pyWolf.system.w_color.GREEN())

        self._shape_circle = pyWolf.graphics.w_shapes(
            pyWolf.glm.vec3(0.0, 0.0, 0.0),
            2.0,
            pyWolf.system.w_color.ORANGE(),
            pyWolf.system.w_plane.XY,
            30)

        _bounding_box = pyWolf.system.w_bounding_box()
        _bounding_box.min = pyWolf.glm.vec3(-3.0, -3.0, -3.0)
        _bounding_box.max = pyWolf.glm.vec3(3.0, 3.0, 3.0)
        self._shape_box = pyWolf.graphics.w_shapes(_bounding_box, pyWolf.system.w_color.YELLOW())

        _bounding_sphere = pyWolf.system.w_bounding_sphere()
        _bounding_sphere.center =  pyWolf.glm.vec3(0.0, 0.0, 0.0)
        _bounding_sphere.radius = 3.0
        _sphere_resolution = 30
        self._shape_sphere = pyWolf.graphics.w_shapes(_bounding_sphere, pyWolf.system.w_color.PURPLE(), _sphere_resolution)
        #++++++++++++++++++++++++++++++++++++++++++++++++++++
        #++++++++++++++++++++++++++++++++++++++++++++++++++++
        
        _config = pyWolf.graphics.w_graphics_device_manager_configs()
        _config.debug_gpu = False
        self._game.set_graphics_device_manager_configs(_config)

    def pre_init(self):
        print "pre_init"

    def post_init(self):
        #get main graphics device
        self._gDevice = self._game.get_graphics_device(0)
        print self._gDevice.get_info()
        print "post_init"

    def load(self):
        #initialize viewport
        self._viewport.y = 0
        self._viewport.width = screen_width
        self._viewport.height = screen_height
        self._viewport.minDepth = 0
        self._viewport.maxDepth = 1
        
        #initialize scissor of viewport
        self._viewport_scissor.offset.x = 0
        self._viewport_scissor.offset.y = 0
        self._viewport_scissor.extent.width = screen_width
        self._viewport_scissor.extent.height = screen_height

        #load render pass which contains frame buffers
        _render_pass_attachments = []
        _output_window = self._gDevice.output_presentation_window
        for _iter in _output_window.swap_chain_image_views:
            # COLOR                                 #DEPTH
            _render_pass_attachments.append([_iter, _output_window.depth_buffer_image_view])

        _hr = self._draw_render_pass.load(self._gDevice, self._viewport, self._viewport_scissor, _render_pass_attachments)
        if _hr:
            print "Error on loading render pass"
            self.release()
            sys.exit(1)

        #create one semaphore for drawing
        _hr = self._draw_semaphore.initialize(self._gDevice)
        if _hr:
            print "Error on initializing semaphore"
            self.release()
            sys.exit(1)

        #create one fence for drawing
        _hr = self._draw_fence.initialize(self._gDevice, 1)
        if _hr:
            print "Error on initializing fence(s)"
            self.release()
            sys.exit(1)

        #++++++++++++++++++++++++++++++++++++++++++++++++++++
        #The following codes have been added for this project
        #++++++++++++++++++++++++++++++++++++++++++++++++++++
        _hr = self._shape_line.load(
            self._gDevice,
            self._draw_render_pass,
            self._viewport,
            self._viewport_scissor)
        if _hr:
            print "Error on loading shape line axis"
            self.release()
            sys.exit(1)

        _hr = self._shape_triangle.load(
            self._gDevice,
            self._draw_render_pass,
            self._viewport,
            self._viewport_scissor)
        if _hr:
            print "Error on loading shape triangle axis"
            self.release()
            sys.exit(1)

        _hr = self._shape_circle.load(
            self._gDevice,
            self._draw_render_pass,
            self._viewport,
            self._viewport_scissor)
        if _hr:
            print "Error on loading shape circle axis"
            self.release()
            sys.exit(1)

        _hr = self._shape_box.load(
            self._gDevice,
            self._draw_render_pass,
            self._viewport,
            self._viewport_scissor)
        if _hr:
            print "Error on loading shape box axis"
            self.release()
            sys.exit(1)

        _hr = self._shape_sphere.load(
            self._gDevice,
            self._draw_render_pass,
            self._viewport,
            self._viewport_scissor)
        if _hr:
            print "Error on loading shape shpere axis"
            self.release()
            sys.exit(1)
        #++++++++++++++++++++++++++++++++++++++++++++++++++++
        #++++++++++++++++++++++++++++++++++++++++++++++++++++

        #create command buffers for drawing
        number_of_swap_chains = self._gDevice.get_number_of_swap_chains()
        _hr = self._draw_command_buffers.load(self._gDevice, number_of_swap_chains, pyWolf.graphics.w_command_buffer_level.PRIMARY)
        if _hr:
            print "Error on initializing draw command buffer(s)"
            self.release()
            sys.exit(1)

        _hr = self.build_command_buffers()
        if _hr:
            print "Error on building command buffers"
            self.release()
            sys.exit(1)
            
        print "scene loaded successfully"

    def build_command_buffers(self):
        _hr = pyWolf.W_PASSED
        _size = self._draw_command_buffers.get_commands_size()
        for i in xrange(_size):
            _cmd = self._draw_command_buffers.get_command_at(i)
            _hr = self._draw_command_buffers.begin(i)
            if _hr:
                print "Error on begining command buffer: " + str(i)
                break
            
            self._draw_render_pass.begin(i, _cmd, pyWolf.system.w_color.CORNFLOWER_BLUE(), 1.0, 0)
            #++++++++++++++++++++++++++++++++++++++++++++++++++++
            #The following codes have been added for this project
            #++++++++++++++++++++++++++++++++++++++++++++++++++++
            self._shape_line.draw(_cmd)
            self._shape_triangle.draw(_cmd)
            self._shape_circle.draw(_cmd)
            self._shape_box.draw(_cmd)
            self._shape_sphere.draw(_cmd)
            #++++++++++++++++++++++++++++++++++++++++++++++++++++
            #++++++++++++++++++++++++++++++++++++++++++++++++++++
            self._draw_render_pass.end(_cmd)
            
            _hr = self._draw_command_buffers.end(i)
            if _hr:
                print "Error on ending command buffer: " + str(i)
                break

        return _hr

    def update(self, pGameTime):
        #Update label of gui widget	
        global _gui
        _gui.debug_text = "FPS: " + str(pGameTime.get_frames_per_second()) + "\r\n\r\nFrameTime: " + str(pGameTime.get_elapsed_seconds()) + "\r\n\r\nTotalTime: " + str(pGameTime.get_total_seconds())
        #++++++++++++++++++++++++++++++++++++++++++++++++++++
        #The following codes have been added for this project
        #++++++++++++++++++++++++++++++++++++++++++++++++++++
        _pi = 3.14159265
        
        _angle = pGameTime.get_total_seconds()
        _eye = pyWolf.glm.vec3(cos(_angle * 0.5) * 15.0, 0.5 * 15.0, sin(_angle * 0.5) * 15.0)
        _up = pyWolf.glm.vec3(0.0, -1.0, 0.0)
        _look_at = pyWolf.glm.vec3(0.0, 0.0, 0.0)

        _world = pyWolf.glm.mat4x4()
        _view = pyWolf.lookAtRH(_eye, _look_at, _up)
        _projection = pyWolf.perspectiveRH(
            45.0 * _pi / 180.0,
            self._viewport.width / self._viewport.height,
            0.1,
            1000.0)
        _vp = pyWolf.multiply_matrices(_projection,_view)
        _wvp = pyWolf.multiply_matrices(_vp,_world)

        _hr = self._shape_line.update(_wvp)
        if _hr:
            print "Error on updating shape line"
        _hr = self._shape_triangle.update(_wvp)
        if _hr:
            print "Error on updating shape triangle"
        _hr = self._shape_circle.update(_wvp)
        if _hr:
            print "Error on updating shape circle"
        _hr = self._shape_box.update(_wvp)
        if _hr:
            print "Error on updating shape box"
        _hr = self._shape_sphere.update(_wvp)
        if _hr:
            print "Error on updating shape sphere"
        #++++++++++++++++++++++++++++++++++++++++++++++++++++
        #++++++++++++++++++++++++++++++++++++++++++++++++++++
	
    def pre_render(self, pGameTime):
        _output_window = self._gDevice.output_presentation_window
        _frame_index = _output_window.swap_chain_image_index

        _wait_dst_stage_mask = [ pyWolf.graphics.w_pipeline_stage_flag_bits.COLOR_ATTACHMENT_OUTPUT_BIT ]
        _wait_semaphores = [ _output_window.swap_chain_image_is_available_semaphore ]
        _signal_semaphores = [ _output_window.rendering_done_semaphore ]
        _cmd = self._draw_command_buffers.get_command_at(_frame_index)

        #reset draw fence
        self._draw_fence.reset()
        _hr = self._gDevice.submit(
            [_cmd], 
            self._gDevice.graphics_queue, 
            _wait_dst_stage_mask, 
            _wait_semaphores, 
            _signal_semaphores, 
            self._draw_fence)
        if _hr:
            print "Error on submiting queue for final drawing"
            return 

        _hr = self._draw_fence.wait()
        if _hr:
            print "Error on waiting for draw fence"
            return 

    
    def post_render(self, pSuccessfullyRendered):
        if pSuccessfullyRendered == False:
            print "Rendered Unsuccessfully"
    
    def run(self):
        #run game
        _window_info = pyWolf.system.w_window_info()
        _window_info.width = self.width()
        _window_info.height = self.height()
        _window_info.v_sync_enable = False
        _window_info.is_full_screen = False
        _window_info.swap_chain_format = 44 # BGRA8Unorm in VULKAN 
        _window_info.cpu_access_swap_chain_buffer = False

        # get window handle
        pycobject_hwnd = self.winId()
        #convert window handle as HWND to unsigned integer pointer for c++
        ctypes.pythonapi.PyCObject_AsVoidPtr.restype  = ctypes.c_void_p
        ctypes.pythonapi.PyCObject_AsVoidPtr.argtypes = [ctypes.py_object]
        int_hwnd = ctypes.pythonapi.PyCObject_AsVoidPtr(pycobject_hwnd)
        _window_info.set_win_id(int_hwnd)
  
        #initialize game
        _map_info = (0, _window_info)
        while True:
            if self.__exiting:
                self.release()
                break
            self._game.run(_map_info)

        print "Game exited"

    def showEvent(self, event):
        #run in another thread
        threading.Thread(target=self.run).start()
        event.accept()

    def closeEvent(self, event):
        self.__exiting = True
        event.accept()

    def keyPressEvent(self, event):
        _key = event.key()
        if _key == QtCore.Qt.Key.Key_Escape:
            self.__exiting = True

    def release(self):
        self._draw_fence.release()
        self._draw_fence = None
        
        self._draw_semaphore.release()
        self._draw_semaphore = None
        
        self._draw_command_buffers.release()
        self._draw_command_buffers = None

        self._draw_render_pass.release()
        self._draw_render_pass = None

        #++++++++++++++++++++++++++++++++++++++++++++++++++++
        #The following codes have been added for this project
        #++++++++++++++++++++++++++++++++++++++++++++++++++++
        self._shape_line.release()
        self._shape_line = None
        
        self._shape_triangle.release()
        self._shape_triangle = None
        
        self._shape_circle.release()
        self._shape_circle = None
        
        self._shape_box.release()
        self._shape_box = None
        
        self._shape_sphere.release()
        self._shape_sphere = None
        #++++++++++++++++++++++++++++++++++++++++++++++++++++
        #++++++++++++++++++++++++++++++++++++++++++++++++++++

        self._game.release()
        self._game = None
        self._gDevice = None
        self._viewport = None
        self._viewport_scissor = None
        
if __name__ == '__main__':
    # Create a Qt application
    _app = QApplication(sys.argv)

    #Init gui
    _gui = gui()
    _gui.resize(screen_width /2, screen_height /2)
    _gui.setWindowTitle('Wolf.Engine Debug')
    
    #Init scene
    _scene = scene(pyWolfPath + "..\\..\\..\\..\\content\\",
                  pyWolfPath,
                  "py_11_pipeline")
    _scene.resize(screen_width, screen_height)
    _scene.setWindowTitle('Wolf.Engine')

    #Show all widgets
    _scene.show()
    _gui.show()

    sys.exit(_app.exec_())
