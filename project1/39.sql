SELECT name
FROM CatchedPokemon, Trainer
WHERE CatchedPokemon.owner_id = Trainer.id
GROUP BY name, pid
HAVING COUNT(*) > 1
ORDER BY name