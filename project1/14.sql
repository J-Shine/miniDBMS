SELECT name
FROM Pokemon, Evolution
WHERE Pokemon.id = Evolution.before_id
AND type = 'Grass'