scoreboard players operation .global.dp.print_score.a local = .global.dp.why.a local
function global:dp/print_score
scoreboard players set .global.dp.why.__is_returned local 1
return 0

