tellraw @a ["score a is ",{ score:{name:".global.dp.compare.a",objective:"local"} }]
tellraw @a ["score b is ",{ score:{name:".global.dp.compare.b",objective:"local"} }]
execute store result score .global.dp.compare.__temp1 local run execute if score .global.dp.compare.a local > .global.dp.compare.b local
scoreboard players operation .global.dp.compare.cmp local = .global.dp.compare.__temp1 local
tellraw @a ["score compared is ",{ score:{name:".global.dp.compare.cmp",objective:"local"} }]

