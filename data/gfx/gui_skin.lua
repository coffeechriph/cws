--require ("lib")
gui_skin = gui.new_skin_table()

gui_skin.button.text_offset         = "center"
gui_skin.button.scale               = vec2(1,1)
gui_skin.button.back_color          = vec3(76, 76, 76)
gui_skin.button.front_color         = vec3(102, 102, 102)
gui_skin.button.outline_color       = vec3(85, 85, 85)

gui_skin.slider.text_offset         = "right_out"
gui_skin.slider.scale               = vec2(1,1)
gui_skin.slider.mark_scale          = vec2(0.05, 1.5)
gui_skin.slider.back_color          = vec3(102, 102, 102)
gui_skin.slider.outline_color       = vec3(85, 85, 85)
gui_skin.slider.front_color         = vec3(127, 127, 127)
gui_skin.slider.mark_back_color     = vec3(127, 127, 127)
gui_skin.slider.mark_front_color    = vec3(178, 178, 178)

gui_skin.checkbox.text_offset       = "center"
gui_skin.checkbox.scale             = vec2(1,1)
gui_skin.checkbox.mark_scale        = vec2(0.1, 0.5)
gui_skin.checkbox.back_color        = vec3(102, 102, 102)
gui_skin.checkbox.outline_color     = vec3(85, 85, 85)
gui_skin.checkbox.mark_back_color   = vec3(127, 127, 127)
gui_skin.checkbox.mark_front_color  = vec3(178, 178, 178)

gui.set_skin(gui_skin)