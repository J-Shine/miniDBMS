SELECT Trainer.name
FROM Trainer, CatchedPokemon
WHERE Trainer.id = CatchedPokemon.owner_id
AND CatchedPokemon.pid IN (
  SELECT id
  FROM Pokemon, Evolution AS e3
  WHERE Pokemon.id = e3.after_id
  AND name NOT IN (
    SELECT name
    FROM Pokemon, Evolution AS e1, Evolution AS e2
    WHERE e1.after_id = Pokemon.id
    AND e2.before_id = e1.after_id
  )
)
ORDER BY Trainer.name ASC