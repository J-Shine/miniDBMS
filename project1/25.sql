SELECT DISTINCT Pokemon.name
FROM Trainer, CatchedPokemon, Pokemon
WHERE Trainer.id = CatchedPokemon.owner_id
AND CatchedPokemon.pid = Pokemon.id
AND Trainer.hometown = 'Sangnok City'
AND Pokemon.id IN (SELECT CatchedPokemon.pid
                             FROM Trainer, CatchedPokemon
                             WHERE Trainer.id = CatchedPokemon.owner_id
                             AND Trainer.hometown = 'Brown City')
ORDER BY Pokemon.name