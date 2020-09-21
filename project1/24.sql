SELECT t.hometown, AVG(level) AS al
FROM Trainer AS t, CatchedPokemon AS cp
WHERE t.id = cp.owner_id
GROUP BY t.hometown
ORDER BY al