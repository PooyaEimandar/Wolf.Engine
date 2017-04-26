/*
	Project			 : Wolf Engine. Copyright(c) Pooya Eimandar (http://PooyaEimandar.com) . All rights reserved.
	Source			 : Please direct any bug to https://github.com/WolfSource/Wolf.Engine/issues
	Website			 : http://WolfSource.io
	Name			 : w_content_manager.h
	Description		 : This content manager class
	Comment          :
*/

#ifndef __W_CONTENT_MANAGER_H__
#define __W_CONTENT_MANAGER_H__

#include "w_cpipeline_export.h"
#include <w_object.h>
#include <string>
#include <w_io.h>
#include "collada/c_parser.h"

namespace wolf
{
	namespace content_pipeline
	{
		class w_content_manager : public wolf::system::w_object
		{
		public:
			
			WCP_EXP w_content_manager();
			WCP_EXP virtual ~w_content_manager();
			
			template<class T>
			T* load(std::wstring pAssetPath)
			{
#if defined(__WIN32) || defined(__UWP)
                auto _path_wcstr = pAssetPath.c_str();
                auto _file_exists = wolf::system::io::get_is_fileW(_path_wcstr);
#else
                auto _path_cstr = wolf::system::convert::wstring_to_string(pAssetPath).c_str();
                auto _file_exists = wolf::system::io::get_is_file(_path_cstr);
#endif
				if (_file_exists  == S_FALSE)
				{
                    logger.error(L"File asset not available on following path : " + pAssetPath);
					return nullptr;
				}
                
                auto _extension = wolf::system::io::get_file_extentionW(pAssetPath);
				//to lower
				std::transform(_extension.begin(), _extension.end(), _extension.begin(), ::tolower);
				std::string _type(typeid(T).name());
                
                if (_extension == L".dae")
                {
                    if (_type == "class wolf::content_pipeline::w_scene")
                    {
                        //load scene from collada file
                        auto _scene = new w_scene();
                        if (collada::c_parser::parse_collada_from_file(pAssetPath, _scene) == S_OK)
                        {
                            return _scene;
                        }
                        return nullptr;
                    }
                }
                
                logger.error(L"Asset not supported : " + pAssetPath);
				
				return nullptr;
			}

		private:
			typedef w_object _super;

			
		};
	}
}

#endif
