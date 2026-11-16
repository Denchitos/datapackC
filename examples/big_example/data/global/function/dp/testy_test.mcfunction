scoreboard players set .global.dp.testy_test.x local 15
scoreboard players operation .global.dp.test_inline.b local = .global.dp.testy_test.x local
scoreboard players operation .global.dp.print_score.a local = .global.dp.test_inline.b local
function global:dp/print_score

