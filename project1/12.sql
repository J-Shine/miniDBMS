SELECT DISTINCT name, type
FROM CatchedPokemon, Pokemon
WHERE CatchedPokemon.pid = Pokemon.id
AND level >= 30
ORDER BY name