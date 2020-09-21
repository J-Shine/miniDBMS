SELECT AVG(level)
FROM CatchedPokemon, Pokemon, Trainer
WHERE CatchedPokemon.owner_id = Trainer.id
AND CatchedPokemon.pid = Pokemon.id
AND Pokemon.type = 'Electric'
AND Trainer.hometown = 'Sangnok City'