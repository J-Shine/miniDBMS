SELECT SUM(level)
FROM CatchedPokemon, Trainer
WHERE CatchedPokemon.owner_id = Trainer.id
AND Trainer.name = 'Matis'