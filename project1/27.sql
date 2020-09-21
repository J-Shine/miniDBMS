SELECT name, MAX(level)
FROM Trainer, CatchedPokemon
WHERE Trainer.id = CatchedPokemon.owner_id
GROUP BY Trainer.name
HAVING COUNT(*) >= 4
ORDER BY name