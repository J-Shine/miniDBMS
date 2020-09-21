SELECT Pokemon.name
FROM Pokemon
WHERE Pokemon.name NOT IN (SELECT Pokemon.name
                                             FROM Pokemon, CatchedPokemon
                                             WHERE Pokemon.id = CatchedPokemon.pid)
ORDER BY Pokemon.name