	section .rodata

	global textures_img
	global textures_cmap
	global textures_slut
	global room_mesh

	align 4
textures_img:
	incbin 'data/tex.img'
textures_cmap:
	incbin 'data/tex.pal'
textures_slut:
	;incbin 'data/tex.slut'

	align 4
room_mesh:
	incbin 'data/room.mesh'


; vi:set ts=8 sts=8 sw=8 ft=nasm:
