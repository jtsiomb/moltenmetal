	section .rodata

	global textures_img
	global textures_cmap
	global textures_slut

	align 4
textures_img:
	incbin 'data/tex.img'
	align 4
textures_cmap:
	incbin 'data/tex.pal'
	align 4
textures_slut:
	incbin 'data/tex.slut'

; vi:set ts=8 sts=8 sw=8 ft=nasm:
