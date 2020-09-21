SELECT COUNT(DISTINCT pid)
FROM CatchedPokemon, Trainer
WHERE CatchedPokemon.owner_id = Trainer.id
AND Trainer.hometown = 'Sangnok City'