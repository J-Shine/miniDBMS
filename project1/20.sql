SELECT Trainer.name, COUNT(owner_id) AS CountPokemon
FROM Trainer, CatchedPokemon
WHERE Trainer.id = CatchedPokemon.owner_id
AND Trainer.hometown = 'Sangnok City'
GROUP BY Trainer.id
ORDER BY CountPokemon ASC