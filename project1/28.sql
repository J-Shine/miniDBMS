SELECT Trainer.name, AVG(level) AS al
FROM CatchedPokemon, Trainer, Pokemon
WHERE CatchedPokemon.owner_id = Trainer.id
AND CatchedPokemon.pid = Pokemon.id
AND (Pokemon.type = 'Electric' OR Pokemon.type = 'Normal')
GROUP BY CatchedPokemon.owner_id
ORDER BY al