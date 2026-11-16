data modify storage item:main a.count set value 5
data modify storage item:main a.name set value "minecraft:diamond"
data modify storage item:main __temp0 set value "Имя предмета:"
tellraw @a { storage : "item:main",nbt:"__temp0",interpret:true }
tellraw @a { storage : "item:main",nbt:"a.name",interpret:true }
data modify storage item:main __temp1 set value "Количество"
tellraw @a { storage : "item:main",nbt:"__temp1",interpret:true }
tellraw @a { storage : "item:main",nbt:"a.count",interpret:false }
data modify storage item:main __temp2 set value "minecraft:emerald"
data modify storage item:main a.name set from storage item:main __temp2
data modify storage item:main __temp3 set value "Имя предмета:"
tellraw @a { storage : "item:main",nbt:"__temp3",interpret:true }
tellraw @a { storage : "item:main",nbt:"a.name",interpret:true }
data modify storage item:main __temp4 set value "Количество"
tellraw @a { storage : "item:main",nbt:"__temp4",interpret:true }
tellraw @a { storage : "item:main",nbt:"a.count",interpret:false }
data modify storage item:main a.count set value 15
data modify storage item:main __temp5 set value "Имя предмета:"
tellraw @a { storage : "item:main",nbt:"__temp5",interpret:true }
tellraw @a { storage : "item:main",nbt:"a.name",interpret:true }
data modify storage item:main __temp6 set value "Количество"
tellraw @a { storage : "item:main",nbt:"__temp6",interpret:true }
tellraw @a { storage : "item:main",nbt:"a.count",interpret:false }
data modify storage item:item/make_1_copy self set from storage item:main a
function item:item/make_1_copy
data modify storage item:main b set from storage item:item/make_1_copy __returned
data modify storage item:main __temp7 set value "Имя предмета:"
tellraw @a { storage : "item:main",nbt:"__temp7",interpret:true }
tellraw @a { storage : "item:main",nbt:"b.name",interpret:true }
data modify storage item:main __temp8 set value "Количество"
tellraw @a { storage : "item:main",nbt:"__temp8",interpret:true }
tellraw @a { storage : "item:main",nbt:"b.count",interpret:false }

