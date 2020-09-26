SELECT t1.id, COUNT(*) AS CountPokemon1
FROM Trainer AS t1, CatchedPokemon AS cp1
WHERE t1.id = cp1.owner_id
GROUP BY t1.id
HAVING CountPokemon1 = (
  SELECT MAX(t.CountPokemon)
  FROM (
    SELECT COUNT(*) AS CountPokemon
    FROM Trainer, CatchedPokemon
    WHERE Trainer.id = CatchedPokemon.owner_id
    GROUP BY Trainer.id) AS t
  )
ORDER BY t1.id ASC