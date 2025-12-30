# Flashing the keyboard:
# From repository root: make -C cantor flash
# Or directly: cd .. && qmk flash -kb cantor -km shofel

.DEFAULT_GOAL := flash

keyboard = cantor
keymap = shofel
repo_root = $(shell git rev-parse --show-toplevel)
keymap_file = ${repo_root}/layouts/default/split_3x6_3/${keymap}/keymap.c

# Flash using QMK CLI (must be run from repo root)
flash:
	@cd ${repo_root} && qmk flash -kb ${keyboard} -km ${keymap}

# Compile using QMK CLI (must be run from repo root)
compile:
	@cd ${repo_root} && qmk compile -kb ${keyboard} -km ${keymap}

visual = visual
json = ${visual}/keymap.json
yaml = ${visual}/keymap.yml
yaml_old = ${visual}/keymap.yaml.old
svg = ${visual}/keymap.svg
combos = ${visual}/combos.yml
draw_config = ${visual}/draw_config.yml

c2json:
	qmk c2json -q ${keymap_file} -kb ${keyboard} -km ${keymap} --no-cpp > ${json}

parse:
	@make c2json
	mv ${yaml} ${yaml_old}
	keymap parse -b ${yaml_old} -q ${json} > ${yaml}
	rm ${yaml_old}

parse_and_fix:
	@make c2json
	keymap parse -c 12 -q ${json} \
		--layer-names Boo Ru Symbols Num Fsys Mouse \
		> ${yaml}
	sd XX "''" ${yaml}
	# BOO home row mods
	sd 'BH A' "{t: 'A', h: 'Super'}" ${yaml}
	sd 'BH O' "{t: 'O', h: 'Alt'}" ${yaml}
	sd 'BH E' "{t: 'E', h: 'Num'}" ${yaml}
	sd 'BH S' "{t: 'S', h: 'Ctrl'}" ${yaml}
	sd 'BH N' "{t: 'N', h: 'Ctrl'}" ${yaml}
	sd 'BH T' "{t: 'T', h: 'Num'}" ${yaml}
	sd 'BH R' "{t: 'R', h: 'Alt'}" ${yaml}
	sd 'BH I' "{t: 'I', h: 'Super'}" ${yaml}
	# Ru home row mods
	sd 'RU ' '' ${yaml}
	sd 'RH F' "{t: 'F', h: 'Super'}" ${yaml}
	sd 'RH Y' "{t: 'Y', h: 'Alt'}" ${yaml}
	sd 'RH V' "{t: 'V', h: 'Num'}" ${yaml}
	sd 'RH A' "{t: 'A', h: 'Ctrl'}" ${yaml}
	sd 'RH O' "{t: 'O', h: 'Ctrl'}" ${yaml}
	sd 'RH L' "{t: 'L', h: 'Num'}" ${yaml}
	sd 'RH D' "{t: 'D', h: 'Alt'}" ${yaml}
	sd 'RH Z' "{t: 'Z', h: 'Super'}" ${yaml}
	#
	sd 'KK[ \n]MOUSE' '{h: Mouse}' ${yaml}
	sd 'KK[ \n]SHIFT' 'Shift' ${yaml}
	sd 'KK[ \n]SYMBO' 'Symbols' ${yaml}
	sd 'KK[ \n]ENTER' '{t: Enter, h: Symbols}' ${yaml}
	sd 'KK[ \n]SPACE' '{t: Space, h: Nav}' ${yaml}
	sd 'KK[ \n]RU' '{t: Ru, h: Fsys}' ${yaml}
	sd 'KK[ \n]NOOP' "''" ${yaml}
	sd 'EM THNK' "🤔" ${yaml}
	cat ${draw_config} >> ${yaml}
	cat ${combos} >> ${yaml}

draw:
	@make parse_and_fix
	keymap draw ${yaml} > ${svg}
	@make view

view:
	firefox ${svg}
