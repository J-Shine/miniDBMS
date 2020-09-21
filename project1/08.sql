SELECT AVG(level)
FROM CatchedPokemon, Trainer
WHERE CatchedPokemon.owner_id = Trainer.id
AND Trainer.name = 'Red'