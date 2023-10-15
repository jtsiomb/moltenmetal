	section .rodata USE32

	global textures_img
	global textures_cmap
	global textures_slut
	global room_mesh
	global _textures_img
	global _textures_cmap
	global _textures_slut
	global _room_mesh

	align 4
textures_img:
_textures_img:
	incbin 'data/tex.img'
textures_cmap:
_textures_cmap:
	incbin 'data/tex.pal'
textures_slut:
_textures_slut:
	;incbin 'data/tex.slut'

	align 4
room_mesh:
_room_mesh:
	incbin 'data/room.mesh'


; vi:set ts=8 sts=8 sw=8 ft=nasm:
