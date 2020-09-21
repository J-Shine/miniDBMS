SELECT DISTINCT t.name
FROM CatchedPokemon AS cp, Trainer AS t
WHERE cp.owner_id = t.id
AND cp.level <= 10
ORDER BY t.name