SELECT name
FROM CatchedPokemon, Pokemon
WHERE CatchedPokemon.pid = Pokemon.id
AND nickname LIKE '% %'
ORDER BY name DESC