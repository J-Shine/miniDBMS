SELECT Pokemon.name, Pokemon.id
FROM Trainer, CatchedPokemon, Pokemon
WHERE Trainer.id = CatchedPokemon.owner_id
AND CatchedPokemon.pid = Pokemon.id
AND Trainer.hometown = 'Sangnok City'
ORDER BY Pokemon.id ASC